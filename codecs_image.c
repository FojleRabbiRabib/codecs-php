/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2026 Fojle Rabbi (Rabib)
 *
 * codecs-php — Codecs\Image implementation, object lifecycle, and pixel accessors.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "zend_exceptions.h"
#include "zend_interfaces.h"
#include "zend_enum.h"
#include "codecs_internal.h"
#include <string.h>
#include <dlfcn.h>

PHP_METHOD(Codecs_Image, fromPixels);
PHP_METHOD(Codecs_Image, fromPng);
PHP_METHOD(Codecs_Image, fromGd);
PHP_METHOD(Codecs_Image, width);
PHP_METHOD(Codecs_Image, height);
PHP_METHOD(Codecs_Image, bitDepth);
PHP_METHOD(Codecs_Image, alphaMode);
PHP_METHOD(Codecs_Image, provenance);
PHP_METHOD(Codecs_Image, cicp);
PHP_METHOD(Codecs_Image, iccProfile);
PHP_METHOD(Codecs_Image, exif);
PHP_METHOD(Codecs_Image, xmp);
PHP_METHOD(Codecs_Image, orientation);
PHP_METHOD(Codecs_Image, memoryUsage);
PHP_METHOD(Codecs_Image, close);
PHP_METHOD(Codecs_Image, toPng);
PHP_METHOD(Codecs_Image, rgba16);
PHP_METHOD(Codecs_Image, rgba8);
PHP_METHOD(Codecs_Image, __clone);
PHP_METHOD(Codecs_Image, __debugInfo);

static zend_object_handlers codecs_image_handlers;

void codecs_image_free(zend_object *obj)
{
	codecs_image_object *intern = codecs_image_from_obj(obj);

	if (intern->pixels) {
		efree(intern->pixels);
		intern->pixels = NULL;
	}
	if (intern->provenance) {
		efree(intern->provenance);
		intern->provenance = NULL;
	}
	if (intern->icc_profile) {
		efree(intern->icc_profile);
		intern->icc_profile = NULL;
	}
	if (intern->exif) {
		efree(intern->exif);
		intern->exif = NULL;
	}
	if (intern->xmp) {
		efree(intern->xmp);
		intern->xmp = NULL;
	}

	zend_object_std_dtor(&intern->std);
}

zend_object *codecs_image_create(zend_class_entry *ce)
{
	codecs_image_object *intern = zend_object_alloc(sizeof(codecs_image_object), ce);

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &codecs_image_handlers;

	intern->pixels = NULL;
	intern->width = 0;
	intern->height = 0;
	intern->bit_depth = 16;
	intern->alpha_mode = 0;
	intern->provenance = NULL;
	memset(&intern->cicp, 0, sizeof(intern->cicp));
	intern->icc_profile = NULL;
	intern->icc_size = 0;
	intern->exif = NULL;
	intern->exif_size = 0;
	intern->xmp = NULL;
	intern->xmp_size = 0;
	intern->orientation = 1;
	intern->closed = false;

	return &intern->std;
}

codecs_image_object *codecs_image_alloc(uint32_t width, uint32_t height, uint32_t bit_depth, uint32_t alpha_mode, const char *provenance)
{
	/* Overflow-checked size computation: width * height * 4 channels * 2 bytes */
	uint64_t pixels_count = (uint64_t)width * (uint64_t)height;
	uint64_t byte_size = pixels_count * 8ULL;

	if (width == 0 || height == 0 || pixels_count > (uint64_t)CODECS_G(max_pixels) || byte_size > (uint64_t)CODECS_G(memory_budget)) {
		return NULL;
	}

	zval zv;
	object_init_ex(&zv, codecs_image_ce);
	codecs_image_object *img = Z_CODECS_IMAGE_P(&zv);

	img->pixels = emalloc(byte_size);
	if (!img->pixels) {
		zval_ptr_dtor(&zv);
		return NULL;
	}

	img->width = width;
	img->height = height;
	img->bit_depth = bit_depth;
	img->alpha_mode = alpha_mode;
	img->provenance = estrdup(provenance ? provenance : "unknown");

	return img;
}

#define CHECK_IMAGE_NOT_CLOSED(img)                                             \
	do {                                                                        \
		if ((img)->closed || !(img)->pixels) {                                  \
			zend_throw_error(NULL, "Cannot operate on a closed Codecs\\Image"); \
			RETURN_THROWS();                                                    \
		}                                                                       \
	} while (0)

ZEND_METHOD(Codecs_Image, fromPixels)
{
	zend_long width, height;
	zend_string *rgba16;
	zval *alpha_zv;

	ZEND_PARSE_PARAMETERS_START(4, 4)
	Z_PARAM_LONG(width)
	Z_PARAM_LONG(height)
	Z_PARAM_STR(rgba16)
	Z_PARAM_OBJECT(alpha_zv)
	ZEND_PARSE_PARAMETERS_END();

	if (width <= 0 || (int64_t)width > CODECS_G(max_dimension)) {
		zend_argument_value_error(1, "must be between 1 and %ld", CODECS_G(max_dimension));
		RETURN_THROWS();
	}

	if (height <= 0 || (int64_t)height > CODECS_G(max_dimension)) {
		zend_argument_value_error(2, "must be between 1 and %ld", CODECS_G(max_dimension));
		RETURN_THROWS();
	}

	uint64_t pixel_count = (uint64_t)width * (uint64_t)height;
	uint64_t expected_bytes = pixel_count * 8ULL;

	if (ZSTR_LEN(rgba16) != expected_bytes) {
		zend_argument_value_error(3, "length (%zu) does not match expected size (%lu bytes for %ldx%ld RGBA16)",
								  ZSTR_LEN(rgba16), expected_bytes, width, height);
		RETURN_THROWS();
	}

	if (pixel_count > (uint64_t)CODECS_G(max_pixels)) {
		codecs_throw(codecs_limit_exception_ce, CODECS_ERR_PIXELS,
					 "Image dimensions (%ldx%ld = %lu pixels) exceed the configured limit (%lu pixels)",
					 width, height, pixel_count, (uint64_t)CODECS_G(max_pixels));
		RETURN_THROWS();
	}

	/* Extract alpha mode from enum case */
	uint32_t alpha_mode = 0;
	zval *case_val = zend_enum_fetch_case_name(Z_OBJ_P(alpha_zv));
	if (case_val && strcmp(Z_STRVAL_P(case_val), "Premultiplied") == 0) {
		alpha_mode = 1;
	}

	object_init_ex(return_value, codecs_image_ce);
	codecs_image_object *img = Z_CODECS_IMAGE_P(return_value);

	img->pixels = emalloc(expected_bytes);
	memcpy(img->pixels, ZSTR_VAL(rgba16), expected_bytes);
	img->width = (uint32_t)width;
	img->height = (uint32_t)height;
	img->bit_depth = 16;
	img->alpha_mode = alpha_mode;
	img->provenance = estrdup("pixels");
}

ZEND_METHOD(Codecs_Image, fromPng)
{
	zend_string *png;

	ZEND_PARSE_PARAMETERS_START(1, 1)
	Z_PARAM_STR(png)
	ZEND_PARSE_PARAMETERS_END();

	codecs_image_object *img = codecs_png_decode((const uint8_t *)ZSTR_VAL(png), ZSTR_LEN(png), NULL);
	if (!img) {
		RETURN_THROWS();
	}

	RETURN_OBJ(&img->std);
}

#define CODECS_GD_MAX_COLORS 256

typedef struct codecs_gd_image_struct {
	unsigned char **pixels;
	int sx;
	int sy;
	int colorsTotal;
	int red[CODECS_GD_MAX_COLORS];
	int green[CODECS_GD_MAX_COLORS];
	int blue[CODECS_GD_MAX_COLORS];
	int open[CODECS_GD_MAX_COLORS];
	int transparent;
	int *polyInts;
	int polyAllocated;
	struct codecs_gd_image_struct *brush;
	struct codecs_gd_image_struct *tile;
	int brushColorMap[CODECS_GD_MAX_COLORS];
	int tileColorMap[CODECS_GD_MAX_COLORS];
	int styleLength;
	int stylePos;
	int *style;
	int interlace;
	int thick;
	int alpha[CODECS_GD_MAX_COLORS];
	int trueColor;
	int **tpixels;
} codecs_gd_image;

typedef codecs_gd_image *(*codecs_gd_fetch_image_fn_t)(zval *);
static codecs_gd_fetch_image_fn_t s_php_gd_fetch_image_fn = (codecs_gd_fetch_image_fn_t)(intptr_t)-1;

ZEND_METHOD(Codecs_Image, fromGd)
{
	zval *gd_zv;

	ZEND_PARSE_PARAMETERS_START(1, 1)
	Z_PARAM_OBJECT(gd_zv)
	ZEND_PARSE_PARAMETERS_END();

	zend_class_entry *gd_ce = zend_hash_str_find_ptr(CG(class_table), "gdimage", sizeof("gdimage") - 1);
	if (!gd_ce) {
		zend_throw_error(NULL, "ext/gd is required to import GD images");
		RETURN_THROWS();
	}

	if (!instanceof_function(Z_OBJCE_P(gd_zv), gd_ce)) {
		zend_type_error("Argument #1 ($gd) must be an instance of GdImage, %s given",
						ZSTR_VAL(Z_OBJCE_P(gd_zv)->name));
		RETURN_THROWS();
	}

	/* Process-level symbol caching with fallback to standard PHP 8 php_image_object struct offset */
	codecs_gd_fetch_image_fn_t fn = (codecs_gd_fetch_image_fn_t)__atomic_load_n(&s_php_gd_fetch_image_fn, __ATOMIC_RELAXED);
	if (fn == (codecs_gd_fetch_image_fn_t)(intptr_t)-1) {
		fn = (codecs_gd_fetch_image_fn_t)dlsym(RTLD_DEFAULT, "php_gd_libgdimageptr_from_zval_p");
		__atomic_store_n(&s_php_gd_fetch_image_fn, fn, __ATOMIC_RELAXED);
	}

	codecs_gd_image *im = NULL;
	if (fn != NULL) {
		im = fn(gd_zv);
	} else {
		im = ((codecs_gd_image **)Z_OBJ_P(gd_zv))[-1];
	}

	if (!im) {
		zend_throw_error(NULL, "Uninitialized or closed GdImage object");
		RETURN_THROWS();
	}

	/* Validate non-positive dimensions on raw signed fields before unsigned cast */
	if (im->sx <= 0 || im->sy <= 0) {
		codecs_throw(codecs_corrupt_data_exception_ce, CODECS_ERR_BITSTREAM,
					 "GD image has invalid non-positive dimensions (%dx%d)", im->sx, im->sy);
		RETURN_THROWS();
	}

	uint32_t width = (uint32_t)im->sx;
	uint32_t height = (uint32_t)im->sy;

	if ((int64_t)width > CODECS_G(max_dimension) || (int64_t)height > CODECS_G(max_dimension)) {
		codecs_throw(codecs_limit_exception_ce, CODECS_ERR_DIMENSION,
					 "GD image dimensions (%ux%u) exceed max dimension limit (%ld)",
					 width, height, CODECS_G(max_dimension));
		RETURN_THROWS();
	}

	uint64_t pixel_count = (uint64_t)width * (uint64_t)height;
	if (pixel_count > (uint64_t)CODECS_G(max_pixels)) {
		codecs_throw(codecs_limit_exception_ce, CODECS_ERR_PIXELS,
					 "GD image pixel count (%llu) exceeds max pixels limit (%llu)",
					 (unsigned long long)pixel_count, (unsigned long long)CODECS_G(max_pixels));
		RETURN_THROWS();
	}

	uint64_t pixel_bytes = pixel_count * 8ULL;
	/* Guard 32-bit SIZE_MAX and configured memory budget */
	if (pixel_bytes > (uint64_t)CODECS_G(memory_budget) || pixel_bytes > (uint64_t)SIZE_MAX) {
		codecs_throw(codecs_limit_exception_ce, CODECS_ERR_LIMIT,
					 "GD image memory requirement (%llu bytes) exceeds memory budget (%ld bytes)",
					 (unsigned long long)pixel_bytes, CODECS_G(memory_budget));
		RETURN_THROWS();
	}

	uint16_t *pixels = emalloc(pixel_bytes);
	bool success = false;
	char err_msg[128] = "";

	/* Bailout safety guard ensuring pixel buffer release on PHP timeout or abort */
	zend_try
	{
		if (im->trueColor) {
			if (!im->tpixels) {
				snprintf(err_msg, sizeof(err_msg), "GD truecolor buffer is not initialized");
			} else {
				success = true;
				for (uint32_t y = 0; y < height; y++) {
					const int *row = im->tpixels[y];
					/* Validate row pointer not NULL */
					if (!row) {
						success = false;
						snprintf(err_msg, sizeof(err_msg), "Corrupted GD truecolor buffer at row %u", y);
						break;
					}
					uint16_t *dst_row = pixels + (size_t)y * width * 4;
					for (uint32_t x = 0; x < width; x++) {
						int p = row[x];
						int a7 = (p >> 24) & 0x7F;
						/* Defensive clamping of alpha values */
						if (a7 < 0) {
							a7 = 0;
						} else if (a7 > 127) {
							a7 = 127;
						}

						uint8_t r8 = (uint8_t)((p >> 16) & 0xFF);
						uint8_t g8 = (uint8_t)((p >> 8) & 0xFF);
						uint8_t b8 = (uint8_t)(p & 0xFF);

						dst_row[x * 4 + 0] = (uint16_t)((r8 << 8) | r8);
						dst_row[x * 4 + 1] = (uint16_t)((g8 << 8) | g8);
						dst_row[x * 4 + 2] = (uint16_t)((b8 << 8) | b8);
						dst_row[x * 4 + 3] = (uint16_t)(((127 - a7) * 65535ULL + 63ULL) / 127ULL);
					}
				}
			}
		} else {
			if (!im->pixels) {
				snprintf(err_msg, sizeof(err_msg), "GD palette buffer is not initialized");
			} else {
				/* Pre-compute 256-entry RGBA16 LUT */
				uint16_t palette_lut[256][4];
				int transparent = im->transparent;
				int total_colors = im->colorsTotal > 256 ? 256 : (im->colorsTotal < 0 ? 0 : im->colorsTotal);

				for (int i = 0; i < 256; i++) {
					if (i < total_colors) {
						uint8_t r8 = (uint8_t)im->red[i];
						uint8_t g8 = (uint8_t)im->green[i];
						uint8_t b8 = (uint8_t)im->blue[i];
						int a7 = (i == transparent) ? 127 : im->alpha[i];
						if (a7 < 0) {
							a7 = 0;
						} else if (a7 > 127) {
							a7 = 127;
						}

						palette_lut[i][0] = (uint16_t)((r8 << 8) | r8);
						palette_lut[i][1] = (uint16_t)((g8 << 8) | g8);
						palette_lut[i][2] = (uint16_t)((b8 << 8) | b8);
						palette_lut[i][3] = (uint16_t)(((127 - a7) * 65535ULL + 63ULL) / 127ULL);
					} else {
						palette_lut[i][0] = 0;
						palette_lut[i][1] = 0;
						palette_lut[i][2] = 0;
						palette_lut[i][3] = 0;
					}
				}

				success = true;
				for (uint32_t y = 0; y < height; y++) {
					const unsigned char *row = im->pixels[y];
					/* Validate row pointer not NULL */
					if (!row) {
						success = false;
						snprintf(err_msg, sizeof(err_msg), "Corrupted GD palette buffer at row %u", y);
						break;
					}
					uint16_t *dst_row = pixels + (size_t)y * width * 4;
					for (uint32_t x = 0; x < width; x++) {
						memcpy(&dst_row[x * 4], palette_lut[row[x]], sizeof(uint16_t) * 4);
					}
				}
			}
		}

		if (success) {
			object_init_ex(return_value, codecs_image_ce);
			codecs_image_object *out = Z_CODECS_IMAGE_P(return_value);
			out->pixels = pixels;
			pixels = NULL; /* Ownership transferred to Zend object dtor */
			out->width = width;
			out->height = height;
			out->bit_depth = 8;
			out->alpha_mode = 0; /* straight */
			out->provenance = estrdup("gd");
			out->orientation = 1; /* TopLeft */
			out->cicp.colour_primaries = 1;
			out->cicp.transfer_characteristics = 13;
			out->cicp.matrix_coefficients = 0;
			out->cicp.full_range = true;
		}
	}
	zend_catch
	{
		if (pixels) {
			efree(pixels);
		}
		zend_bailout();
	}
	zend_end_try();

	if (!success) {
		if (pixels) {
			efree(pixels);
		}
		codecs_throw(codecs_corrupt_data_exception_ce, CODECS_ERR_BITSTREAM,
					 "%s", err_msg[0] ? err_msg : "Failed to import GD image");
		RETURN_THROWS();
	}
}

ZEND_METHOD(Codecs_Image, width)
{
	ZEND_PARSE_PARAMETERS_NONE();
	codecs_image_object *img = Z_CODECS_IMAGE_P(ZEND_THIS);
	CHECK_IMAGE_NOT_CLOSED(img);
	RETURN_LONG(img->width);
}

ZEND_METHOD(Codecs_Image, height)
{
	ZEND_PARSE_PARAMETERS_NONE();
	codecs_image_object *img = Z_CODECS_IMAGE_P(ZEND_THIS);
	CHECK_IMAGE_NOT_CLOSED(img);
	RETURN_LONG(img->height);
}

ZEND_METHOD(Codecs_Image, bitDepth)
{
	ZEND_PARSE_PARAMETERS_NONE();
	codecs_image_object *img = Z_CODECS_IMAGE_P(ZEND_THIS);
	CHECK_IMAGE_NOT_CLOSED(img);
	RETURN_LONG(img->bit_depth);
}

ZEND_METHOD(Codecs_Image, alphaMode)
{
	ZEND_PARSE_PARAMETERS_NONE();
	codecs_image_object *img = Z_CODECS_IMAGE_P(ZEND_THIS);
	CHECK_IMAGE_NOT_CLOSED(img);

	const char *case_name = (img->alpha_mode == 1) ? "Premultiplied" : "Straight";
	zend_object *case_obj = zend_enum_get_case_cstr(codecs_alpha_mode_ce, case_name);
	if (case_obj) {
		RETURN_OBJ_COPY(case_obj);
	}
	RETURN_NULL();
}

ZEND_METHOD(Codecs_Image, provenance)
{
	ZEND_PARSE_PARAMETERS_NONE();
	codecs_image_object *img = Z_CODECS_IMAGE_P(ZEND_THIS);
	CHECK_IMAGE_NOT_CLOSED(img);
	RETURN_STRING(img->provenance ? img->provenance : "unknown");
}

ZEND_METHOD(Codecs_Image, cicp)
{
	ZEND_PARSE_PARAMETERS_NONE();
	codecs_image_object *img = Z_CODECS_IMAGE_P(ZEND_THIS);
	CHECK_IMAGE_NOT_CLOSED(img);

	object_init_ex(return_value, codecs_cicp_ce);
	zend_update_property_long(codecs_cicp_ce, Z_OBJ_P(return_value), "colourPrimaries", sizeof("colourPrimaries") - 1, img->cicp.colour_primaries);
	zend_update_property_long(codecs_cicp_ce, Z_OBJ_P(return_value), "transferCharacteristics", sizeof("transferCharacteristics") - 1, img->cicp.transfer_characteristics);
	zend_update_property_long(codecs_cicp_ce, Z_OBJ_P(return_value), "matrixCoefficients", sizeof("matrixCoefficients") - 1, img->cicp.matrix_coefficients);
	zend_update_property_bool(codecs_cicp_ce, Z_OBJ_P(return_value), "fullRange", sizeof("fullRange") - 1, img->cicp.full_range);
}

ZEND_METHOD(Codecs_Image, iccProfile)
{
	ZEND_PARSE_PARAMETERS_NONE();
	codecs_image_object *img = Z_CODECS_IMAGE_P(ZEND_THIS);
	CHECK_IMAGE_NOT_CLOSED(img);

	if (img->icc_profile && img->icc_size > 0) {
		RETURN_STRINGL((char *)img->icc_profile, img->icc_size);
	}
	RETURN_NULL();
}

ZEND_METHOD(Codecs_Image, exif)
{
	ZEND_PARSE_PARAMETERS_NONE();
	codecs_image_object *img = Z_CODECS_IMAGE_P(ZEND_THIS);
	CHECK_IMAGE_NOT_CLOSED(img);

	if (img->exif && img->exif_size > 0) {
		RETURN_STRINGL((char *)img->exif, img->exif_size);
	}
	RETURN_NULL();
}

ZEND_METHOD(Codecs_Image, xmp)
{
	ZEND_PARSE_PARAMETERS_NONE();
	codecs_image_object *img = Z_CODECS_IMAGE_P(ZEND_THIS);
	CHECK_IMAGE_NOT_CLOSED(img);

	if (img->xmp && img->xmp_size > 0) {
		RETURN_STRINGL((char *)img->xmp, img->xmp_size);
	}
	RETURN_NULL();
}

ZEND_METHOD(Codecs_Image, orientation)
{
	ZEND_PARSE_PARAMETERS_NONE();
	codecs_image_object *img = Z_CODECS_IMAGE_P(ZEND_THIS);
	CHECK_IMAGE_NOT_CLOSED(img);

	zend_object *case_obj = NULL;
	if (zend_enum_get_case_by_value(&case_obj, codecs_orientation_ce, img->orientation, NULL, false) == SUCCESS && case_obj) {
		RETURN_OBJ_COPY(case_obj);
	}
	RETURN_NULL();
}

ZEND_METHOD(Codecs_Image, memoryUsage)
{
	ZEND_PARSE_PARAMETERS_NONE();
	codecs_image_object *img = Z_CODECS_IMAGE_P(ZEND_THIS);

	if (img->closed || !img->pixels) {
		RETURN_LONG(0);
	}

	uint64_t total = (uint64_t)img->width * (uint64_t)img->height * 8ULL;
	total += img->icc_size + img->exif_size + img->xmp_size;
	if (total > ZEND_LONG_MAX) {
		RETURN_LONG(ZEND_LONG_MAX);
	}
	RETURN_LONG((zend_long)total);
}

ZEND_METHOD(Codecs_Image, close)
{
	ZEND_PARSE_PARAMETERS_NONE();
	(void)return_value;
	codecs_image_object *img = Z_CODECS_IMAGE_P(ZEND_THIS);

	if (!img->closed) {
		if (img->pixels) {
			efree(img->pixels);
			img->pixels = NULL;
		}
		if (img->icc_profile) {
			efree(img->icc_profile);
			img->icc_profile = NULL;
			img->icc_size = 0;
		}
		if (img->exif) {
			efree(img->exif);
			img->exif = NULL;
			img->exif_size = 0;
		}
		if (img->xmp) {
			efree(img->xmp);
			img->xmp = NULL;
			img->xmp_size = 0;
		}
		img->closed = true;
	}
}

ZEND_METHOD(Codecs_Image, toPng)
{
	ZEND_PARSE_PARAMETERS_NONE();
	codecs_image_object *img = Z_CODECS_IMAGE_P(ZEND_THIS);
	CHECK_IMAGE_NOT_CLOSED(img);

	zend_string *png = codecs_png_encode(img);
	if (!png) {
		RETURN_THROWS();
	}
	RETURN_STR(png);
}

ZEND_METHOD(Codecs_Image, rgba16)
{
	ZEND_PARSE_PARAMETERS_NONE();
	codecs_image_object *img = Z_CODECS_IMAGE_P(ZEND_THIS);
	CHECK_IMAGE_NOT_CLOSED(img);

	size_t len = (size_t)img->width * (size_t)img->height * 8;
	RETURN_STRINGL((char *)img->pixels, len);
}

ZEND_METHOD(Codecs_Image, rgba8)
{
	ZEND_PARSE_PARAMETERS_NONE();
	codecs_image_object *img = Z_CODECS_IMAGE_P(ZEND_THIS);
	CHECK_IMAGE_NOT_CLOSED(img);

	size_t pixel_count = (size_t)img->width * (size_t)img->height;
	size_t len = pixel_count * 4;

	zend_string *s = zend_string_alloc(len, 0);
	uint8_t *dst = (uint8_t *)ZSTR_VAL(s);
	const uint16_t *src = img->pixels;

	/* Downscale 16-bit to 8-bit: shift right by 8 bits */
	for (size_t i = 0; i < pixel_count * 4; i++) {
		dst[i] = (uint8_t)(src[i] >> 8);
	}
	dst[len] = '\0';

	RETURN_STR(s);
}

ZEND_METHOD(Codecs_Image, __clone)
{
	ZEND_PARSE_PARAMETERS_NONE();
	(void)return_value;
	zend_throw_error(NULL, "Trying to clone an uncloneable object of class Codecs\\Image");
}

ZEND_METHOD(Codecs_Image, __debugInfo)
{
	ZEND_PARSE_PARAMETERS_NONE();
	codecs_image_object *img = Z_CODECS_IMAGE_P(ZEND_THIS);

	array_init(return_value);
	add_assoc_long(return_value, "width", img->width);
	add_assoc_long(return_value, "height", img->height);
	add_assoc_long(return_value, "bitDepth", img->bit_depth);
	add_assoc_string(return_value, "provenance", img->provenance ? img->provenance : "unknown");
	add_assoc_bool(return_value, "closed", img->closed);
	if (!img->closed && img->pixels) {
		add_assoc_long(return_value, "byteSize", (zend_long)((uint64_t)img->width * (uint64_t)img->height * 8ULL));
	}
}

void codecs_image_register_handlers(void)
{
	memcpy(&codecs_image_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	codecs_image_handlers.offset = XtOffsetOf(codecs_image_object, std);
	codecs_image_handlers.free_obj = codecs_image_free;
	codecs_image_handlers.clone_obj = NULL;
}

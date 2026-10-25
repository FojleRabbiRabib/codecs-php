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

	(void)png;
	(void)return_value;
	codecs_throw(codecs_unsupported_format_exception_ce, CODECS_ERR_UNSUPPORTED,
				 "fromPng() is not yet implemented in this milestone");
}

ZEND_METHOD(Codecs_Image, fromGd)
{
	zval *gd;

	ZEND_PARSE_PARAMETERS_START(1, 1)
	Z_PARAM_OBJECT(gd)
	ZEND_PARSE_PARAMETERS_END();

	(void)gd;
	(void)return_value;
	codecs_throw(codecs_unsupported_format_exception_ce, CODECS_ERR_UNSUPPORTED,
				 "fromGd() is not yet implemented in this milestone");
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
	(void)return_value;
	codecs_image_object *img = Z_CODECS_IMAGE_P(ZEND_THIS);
	CHECK_IMAGE_NOT_CLOSED(img);

	codecs_throw(codecs_unsupported_format_exception_ce, CODECS_ERR_UNSUPPORTED,
				 "toPng() is not yet implemented in this milestone");
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

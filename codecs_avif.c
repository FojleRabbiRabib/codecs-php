/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2026 Fojle Rabbi (Rabib)
 *
 * codecs-php — AVIF decoding using vendored libavif + dav1d.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "zend_exceptions.h"
#include "zend_enum.h"
#include "codecs_internal.h"
#include "avif/avif.h"
#include <string.h>

/* Map AVIF irot and imir transform flags to EXIF orientation 1..8 (B3) */
static uint32_t calc_exif_orientation(avifTransformFlags flags, avifImageRotation irot, avifImageMirror imir)
{
	if (!(flags & AVIF_TRANSFORM_IROT) || irot.angle == 0) {
		if (!(flags & AVIF_TRANSFORM_IMIR)) {
			return 1;
		}
		if (imir.axis == 0) {
			return 4;
		}
		return 2;
	}

	if (irot.angle == 1) {
		if (!(flags & AVIF_TRANSFORM_IMIR)) {
			return 8;
		}
		if (imir.axis == 0) {
			return 5;
		}
		return 7;
	}

	if (irot.angle == 2) {
		if (!(flags & AVIF_TRANSFORM_IMIR)) {
			return 3;
		}
		if (imir.axis == 0) {
			return 2;
		}
		return 4;
	}

	/* irot.angle == 3 */
	if (!(flags & AVIF_TRANSFORM_IMIR)) {
		return 6;
	}
	if (imir.axis == 0) {
		return 7;
	}
	return 5;
}

/* Helper: apply orientation transform to 16-bit RGBA buffer (extension-owned, R2-D3 & B2) */
static void apply_orientation_transform(uint16_t **pixels_ptr, uint32_t *width_ptr, uint32_t *height_ptr, avifTransformFlags flags, avifImageRotation irot, avifImageMirror imir)
{
	uint32_t w = *width_ptr;
	uint32_t h = *height_ptr;
	uint16_t *src = *pixels_ptr;

	if (!(flags & (AVIF_TRANSFORM_IROT | AVIF_TRANSFORM_IMIR))) {
		return;
	}

	/* 180 degree rotation in place (B2: decoupled from IMIR) */
	if ((flags & AVIF_TRANSFORM_IROT) && irot.angle == 2) {
		size_t total_pixels = (size_t)w * (size_t)h;
		for (size_t i = 0; i < total_pixels / 2; i++) {
			size_t j = total_pixels - 1 - i;
			for (int c = 0; c < 4; c++) {
				uint16_t tmp = src[i * 4 + c];
				src[i * 4 + c] = src[j * 4 + c];
				src[j * 4 + c] = tmp;
			}
		}
	}

	/* 90 or 270 degree rotation requires dimension swap and new buffer */
	if ((flags & AVIF_TRANSFORM_IROT) && (irot.angle == 1 || irot.angle == 3)) {
		uint32_t new_w = h;
		uint32_t new_h = w;
		size_t new_bytes = (size_t)new_w * (size_t)new_h * 8;
		uint16_t *dst = emalloc(new_bytes);

		for (uint32_t y = 0; y < h; y++) {
			for (uint32_t x = 0; x < w; x++) {
				uint32_t dst_x, dst_y;
				if (irot.angle == 3) {
					/* 90 deg clockwise */
					dst_x = h - 1 - y;
					dst_y = x;
				} else {
					/* 270 deg clockwise */
					dst_x = y;
					dst_y = w - 1 - x;
				}
				size_t src_idx = ((size_t)y * w + x) * 4;
				size_t dst_idx = ((size_t)dst_y * new_w + dst_x) * 4;
				for (int c = 0; c < 4; c++) {
					dst[dst_idx + c] = src[src_idx + c];
				}
			}
		}

		efree(src);
		*pixels_ptr = dst;
		*width_ptr = new_w;
		*height_ptr = new_h;
		src = dst;
		w = new_w;
		h = new_h;
	}

	/* Mirroring */
	if (flags & AVIF_TRANSFORM_IMIR) {
		if (imir.axis == 1) {
			/* Horizontal flip: left and right parts exchanged */
			for (uint32_t y = 0; y < h; y++) {
				for (uint32_t x = 0; x < w / 2; x++) {
					size_t i = ((size_t)y * w + x) * 4;
					size_t j = ((size_t)y * w + (w - 1 - x)) * 4;
					for (int c = 0; c < 4; c++) {
						uint16_t tmp = src[i + c];
						src[i + c] = src[j + c];
						src[j + c] = tmp;
					}
				}
			}
		} else {
			/* Vertical flip: top and bottom parts exchanged */
			for (uint32_t y = 0; y < h / 2; y++) {
				for (uint32_t x = 0; x < w; x++) {
					size_t i = ((size_t)y * w + x) * 4;
					size_t j = ((size_t)(h - 1 - y) * w + x) * 4;
					for (int c = 0; c < 4; c++) {
						uint16_t tmp = src[i + c];
						src[i + c] = src[j + c];
						src[j + c] = tmp;
					}
				}
			}
		}
	}
}

codecs_image_object *codecs_avif_decode(const uint8_t *data, size_t size, zval *options_zv)
{
	if (!data || size == 0) {
		codecs_throw(codecs_corrupt_data_exception_ce, CODECS_ERR_BITSTREAM, "Cannot decode empty or zero-byte data");
		return NULL;
	}

	if (size > (size_t)CODECS_G(max_input_bytes)) {
		codecs_throw(codecs_limit_exception_ce, CODECS_ERR_LIMIT,
					 "Input data (%zu bytes) exceeds configured limit (%zu bytes)", size, (size_t)CODECS_G(max_input_bytes));
		return NULL;
	}

	/* Parse options if provided */
	bool first_frame_only = false;
	bool keep_orientation = false;
	bool ignore_icc = false;
	bool ignore_exif = false;
	bool ignore_xmp = false;
	int64_t max_dim = CODECS_G(max_dimension);
	int64_t max_pix = CODECS_G(max_pixels);
	int threads = (int)CODECS_G(threads);
	int matrix_override = -1;

	if (options_zv && Z_TYPE_P(options_zv) == IS_OBJECT) {
		zval rv;
		zval *prop;

		prop = zend_read_property(codecs_decode_options_ce, Z_OBJ_P(options_zv), "firstFrameOnly", sizeof("firstFrameOnly") - 1, 1, &rv);
		if (prop && Z_TYPE_P(prop) == IS_TRUE) {
			first_frame_only = true;
		}

		prop = zend_read_property(codecs_decode_options_ce, Z_OBJ_P(options_zv), "keepOrientation", sizeof("keepOrientation") - 1, 1, &rv);
		if (prop && Z_TYPE_P(prop) == IS_TRUE) {
			keep_orientation = true;
		}

		prop = zend_read_property(codecs_decode_options_ce, Z_OBJ_P(options_zv), "ignoreIcc", sizeof("ignoreIcc") - 1, 1, &rv);
		if (prop && Z_TYPE_P(prop) == IS_TRUE) {
			ignore_icc = true;
		}

		prop = zend_read_property(codecs_decode_options_ce, Z_OBJ_P(options_zv), "ignoreExif", sizeof("ignoreExif") - 1, 1, &rv);
		if (prop && Z_TYPE_P(prop) == IS_TRUE) {
			ignore_exif = true;
		}

		prop = zend_read_property(codecs_decode_options_ce, Z_OBJ_P(options_zv), "ignoreXmp", sizeof("ignoreXmp") - 1, 1, &rv);
		if (prop && Z_TYPE_P(prop) == IS_TRUE) {
			ignore_xmp = true;
		}

		prop = zend_read_property(codecs_decode_options_ce, Z_OBJ_P(options_zv), "maxDimension", sizeof("maxDimension") - 1, 1, &rv);
		if (prop && Z_TYPE_P(prop) == IS_LONG && Z_LVAL_P(prop) > 0) {
			if (Z_LVAL_P(prop) < max_dim) {
				max_dim = Z_LVAL_P(prop);
			}
		}

		prop = zend_read_property(codecs_decode_options_ce, Z_OBJ_P(options_zv), "maxPixels", sizeof("maxPixels") - 1, 1, &rv);
		if (prop && Z_TYPE_P(prop) == IS_LONG && Z_LVAL_P(prop) > 0) {
			if (Z_LVAL_P(prop) < max_pix) {
				max_pix = Z_LVAL_P(prop);
			}
		}

		prop = zend_read_property(codecs_decode_options_ce, Z_OBJ_P(options_zv), "threads", sizeof("threads") - 1, 1, &rv);
		if (prop && Z_TYPE_P(prop) == IS_LONG && Z_LVAL_P(prop) > 0) {
			threads = (int)Z_LVAL_P(prop);
		}

		prop = zend_read_property(codecs_decode_options_ce, Z_OBJ_P(options_zv), "matrixOverride", sizeof("matrixOverride") - 1, 1, &rv);
		if (prop && Z_TYPE_P(prop) == IS_OBJECT) {
			zval rv_val;
			zval *val = zend_read_property(codecs_matrix_coefficients_ce, Z_OBJ_P(prop), "value", sizeof("value") - 1, 1, &rv_val);
			if (val && Z_TYPE_P(val) == IS_LONG) {
				matrix_override = (int)Z_LVAL_P(val);
			}
		}
	}

	avifDecoder *decoder = avifDecoderCreate();
	if (!decoder) {
		codecs_throw(codecs_decode_exception_ce, CODECS_ERR_OUT_OF_MEMORY, "Failed to allocate AVIF decoder");
		return NULL;
	}

	/* B5: Configure libavif native limits and ignore flags before parsing */
	decoder->maxThreads = threads > 0 ? threads : 1;
	decoder->strictFlags = AVIF_STRICT_ENABLED;
	decoder->imageDimensionLimit = (uint32_t)max_dim;
	decoder->imageSizeLimit = (uint32_t)max_pix;
	decoder->imageCountLimit = first_frame_only ? (uint32_t)CODECS_G(max_frames) : 2;
	decoder->ignoreExif = ignore_exif ? AVIF_TRUE : AVIF_FALSE;
	decoder->ignoreXMP = ignore_xmp ? AVIF_TRUE : AVIF_FALSE;

	avifResult result = avifDecoderSetIOMemory(decoder, data, size);
	if (result != AVIF_RESULT_OK) {
		avifDecoderDestroy(decoder);
		codecs_throw(codecs_corrupt_data_exception_ce, CODECS_ERR_CONTAINER, "Failed to set AVIF memory buffer: %s", avifResultToString(result));
		return NULL;
	}

	result = avifDecoderParse(decoder);
	if (result != AVIF_RESULT_OK) {
		avifDecoderDestroy(decoder);
		codecs_throw(codecs_corrupt_data_exception_ce, CODECS_ERR_CONTAINER, "Failed to parse AVIF container: %s", avifResultToString(result));
		return NULL;
	}

	/* Check animation fail-closed (design record §0) */
	if (decoder->imageCount > 1 && !first_frame_only) {
		int count = decoder->imageCount;
		avifDecoderDestroy(decoder);
		codecs_throw(codecs_unsupported_format_exception_ce, CODECS_ERR_UNSUPPORTED,
					 "Animated AVIF (%d frames) is not supported unless firstFrameOnly option is true", count);
		return NULL;
	}

	/* Decode first frame with zend_try / zend_catch cleanup guard (B6) */
	avifDecoder *volatile volatile_decoder = decoder;
	uint16_t *volatile volatile_pixels = NULL;
	uint16_t *pixels = NULL;
	avifImage *image = NULL;
	uint32_t w = 0, h = 0;

	zend_try
	{
		result = avifDecoderNextImage(decoder);
	}
	zend_catch
	{
		if (volatile_pixels) {
			efree((void *)volatile_pixels);
		}
		if (volatile_decoder) {
			avifDecoderDestroy((avifDecoder *)volatile_decoder);
		}
		zend_bailout();
	}
	zend_end_try();

	if (result != AVIF_RESULT_OK) {
		avifDecoderDestroy(decoder);
		codecs_throw(codecs_decode_exception_ce, CODECS_ERR_BITSTREAM, "Failed to decode AVIF image: %s", avifResultToString(result));
		return NULL;
	}

	image = decoder->image;
	w = image->width;
	h = image->height;
	uint64_t total_pixels = (uint64_t)w * (uint64_t)h;

	/* Extension-level ceiling checks */
	if (w == 0 || h == 0 || (int64_t)w > max_dim || (int64_t)h > max_dim) {
		avifDecoderDestroy(decoder);
		codecs_throw(codecs_limit_exception_ce, CODECS_ERR_DIMENSION,
					 "Image dimensions (%ux%u) exceed max dimension limit (%ld)", w, h, max_dim);
		return NULL;
	}

	if (total_pixels > (uint64_t)max_pix) {
		avifDecoderDestroy(decoder);
		codecs_throw(codecs_limit_exception_ce, CODECS_ERR_PIXELS,
					 "Image pixel count (%lu) exceeds max pixels limit (%ld)", total_pixels, max_pix);
		return NULL;
	}

	/* S4: apply matrixOverride if specified */
	if (matrix_override >= 0) {
		image->matrixCoefficients = (avifMatrixCoefficients)matrix_override;
	}

	/* Setup RGB output: 16-bit depth (full scale per R4-S1), straight alpha per R1-D1 */
	avifRGBImage rgb;
	avifRGBImageSetDefaults(&rgb, image);
	rgb.depth = 16;
	rgb.format = AVIF_RGB_FORMAT_RGBA;
	rgb.alphaPremultiplied = AVIF_FALSE;

	size_t buffer_size = (size_t)w * (size_t)h * 8;
	pixels = emalloc(buffer_size);
	if (!pixels) {
		avifDecoderDestroy(decoder);
		codecs_throw(codecs_decode_exception_ce, CODECS_ERR_OUT_OF_MEMORY, "Failed to allocate %zu bytes for image buffer", buffer_size);
		return NULL;
	}
	volatile_pixels = pixels;

	rgb.pixels = (uint8_t *)pixels;
	rgb.rowBytes = w * 8;

	zend_try
	{
		result = avifImageYUVToRGB(image, &rgb);
	}
	zend_catch
	{
		if (volatile_pixels) {
			efree((void *)volatile_pixels);
		}
		if (volatile_decoder) {
			avifDecoderDestroy((avifDecoder *)volatile_decoder);
		}
		zend_bailout();
	}
	zend_end_try();

	if (result != AVIF_RESULT_OK) {
		efree(pixels);
		avifDecoderDestroy(decoder);
		codecs_throw(codecs_decode_exception_ce, CODECS_ERR_BITSTREAM, "Failed to convert AVIF YUV to RGB: %s", avifResultToString(result));
		return NULL;
	}

	/* B4: Validate metadata limits before object allocation */
	if (!ignore_icc && image->icc.size > (size_t)CODECS_G(max_metadata_bytes)) {
		efree(pixels);
		avifDecoderDestroy(decoder);
		codecs_throw(codecs_limit_exception_ce, CODECS_ERR_METADATA,
					 "ICC profile size (%zu bytes) exceeds limit of %zu bytes", image->icc.size, (size_t)CODECS_G(max_metadata_bytes));
		return NULL;
	}
	if (!ignore_exif && image->exif.size > (size_t)CODECS_G(max_metadata_bytes)) {
		efree(pixels);
		avifDecoderDestroy(decoder);
		codecs_throw(codecs_limit_exception_ce, CODECS_ERR_METADATA,
					 "EXIF payload size (%zu bytes) exceeds limit of %zu bytes", image->exif.size, (size_t)CODECS_G(max_metadata_bytes));
		return NULL;
	}
	if (!ignore_xmp && image->xmp.size > (size_t)CODECS_G(max_metadata_bytes)) {
		efree(pixels);
		avifDecoderDestroy(decoder);
		codecs_throw(codecs_limit_exception_ce, CODECS_ERR_METADATA,
					 "XMP payload size (%zu bytes) exceeds limit of %zu bytes", image->xmp.size, (size_t)CODECS_G(max_metadata_bytes));
		return NULL;
	}

	/* Apply orientation if not disabled (extension-owned transform, R2-D3 & B2) */
	if (!keep_orientation && (image->transformFlags & (AVIF_TRANSFORM_IROT | AVIF_TRANSFORM_IMIR))) {
		apply_orientation_transform(&pixels, &w, &h, image->transformFlags, image->irot, image->imir);
	}

	/* Allocate Image object */
	zval img_zv;
	object_init_ex(&img_zv, codecs_image_ce);
	codecs_image_object *img = Z_CODECS_IMAGE_P(&img_zv);

	img->pixels = pixels;
	img->width = w;
	img->height = h;
	img->bit_depth = image->depth;
	img->alpha_mode = 0; /* straight */
	img->provenance = estrdup("avif");

	/* B3: Calculate and set source orientation */
	img->orientation = calc_exif_orientation(image->transformFlags, image->irot, image->imir);

	/* Populate CICP */
	img->cicp.colour_primaries = (uint32_t)image->colorPrimaries;
	img->cicp.transfer_characteristics = (uint32_t)image->transferCharacteristics;
	img->cicp.matrix_coefficients = (uint32_t)image->matrixCoefficients;
	img->cicp.full_range = (image->yuvRange == AVIF_RANGE_FULL);

	/* Copy ICC if present and not ignored */
	if (!ignore_icc && image->icc.size > 0) {
		img->icc_profile = emalloc(image->icc.size);
		memcpy(img->icc_profile, image->icc.data, image->icc.size);
		img->icc_size = image->icc.size;
	}

	/* Copy EXIF if present and not ignored */
	if (!ignore_exif && image->exif.size > 0) {
		img->exif = emalloc(image->exif.size);
		memcpy(img->exif, image->exif.data, image->exif.size);
		img->exif_size = image->exif.size;
	}

	/* Copy XMP if present and not ignored */
	if (!ignore_xmp && image->xmp.size > 0) {
		img->xmp = emalloc(image->xmp.size);
		memcpy(img->xmp, image->xmp.data, image->xmp.size);
		img->xmp_size = image->xmp.size;
	}

	avifDecoderDestroy(decoder);
	return img;
}

bool codecs_avif_probe(const uint8_t *data, size_t size, zval *return_value)
{
	if (!data || size == 0) {
		codecs_throw(codecs_corrupt_data_exception_ce, CODECS_ERR_BITSTREAM, "Cannot probe empty or zero-byte data");
		return false;
	}

	if (size > (size_t)CODECS_G(max_input_bytes)) {
		codecs_throw(codecs_limit_exception_ce, CODECS_ERR_LIMIT,
					 "Input data (%zu bytes) exceeds configured limit (%zu bytes)", size, (size_t)CODECS_G(max_input_bytes));
		return false;
	}

	avifDecoder *decoder = avifDecoderCreate();
	if (!decoder) {
		codecs_throw(codecs_decode_exception_ce, CODECS_ERR_OUT_OF_MEMORY, "Failed to allocate AVIF decoder");
		return false;
	}

	decoder->maxThreads = 1;
	decoder->strictFlags = AVIF_STRICT_ENABLED;

	avifResult result = avifDecoderSetIOMemory(decoder, data, size);
	if (result != AVIF_RESULT_OK) {
		avifDecoderDestroy(decoder);
		codecs_throw(codecs_corrupt_data_exception_ce, CODECS_ERR_CONTAINER, "Failed to set AVIF memory buffer: %s", avifResultToString(result));
		return false;
	}

	result = avifDecoderParse(decoder);
	if (result != AVIF_RESULT_OK) {
		avifDecoderDestroy(decoder);
		codecs_throw(codecs_corrupt_data_exception_ce, CODECS_ERR_CONTAINER, "Failed to parse AVIF container: %s", avifResultToString(result));
		return false;
	}

	avifImage *image = decoder->image;

	/* Create Codecs\Info object */
	object_init_ex(return_value, codecs_info_ce);

	/* format: Format::Avif */
	zend_object *format_obj = zend_enum_get_case_cstr(codecs_format_ce, "Avif");
	if (format_obj) {
		zval zv;
		ZVAL_OBJ(&zv, format_obj);
		zend_update_property(codecs_info_ce, Z_OBJ_P(return_value), "format", sizeof("format") - 1, &zv);
	}

	zend_update_property_long(codecs_info_ce, Z_OBJ_P(return_value), "width", sizeof("width") - 1, image->width);
	zend_update_property_long(codecs_info_ce, Z_OBJ_P(return_value), "height", sizeof("height") - 1, image->height);
	zend_update_property_long(codecs_info_ce, Z_OBJ_P(return_value), "bitDepth", sizeof("bitDepth") - 1, image->depth);
	zend_update_property_bool(codecs_info_ce, Z_OBJ_P(return_value), "alpha", sizeof("alpha") - 1, decoder->alphaPresent);

	/* Cicp object */
	zval cicp_zv;
	object_init_ex(&cicp_zv, codecs_cicp_ce);
	zend_update_property_long(codecs_cicp_ce, Z_OBJ(cicp_zv), "colourPrimaries", sizeof("colourPrimaries") - 1, image->colorPrimaries);
	zend_update_property_long(codecs_cicp_ce, Z_OBJ(cicp_zv), "transferCharacteristics", sizeof("transferCharacteristics") - 1, image->transferCharacteristics);
	zend_update_property_long(codecs_cicp_ce, Z_OBJ(cicp_zv), "matrixCoefficients", sizeof("matrixCoefficients") - 1, image->matrixCoefficients);
	zend_update_property_bool(codecs_cicp_ce, Z_OBJ(cicp_zv), "fullRange", sizeof("fullRange") - 1, (image->yuvRange == AVIF_RANGE_FULL));
	zend_update_property(codecs_info_ce, Z_OBJ_P(return_value), "cicp", sizeof("cicp") - 1, &cicp_zv);
	zval_ptr_dtor(&cicp_zv);

	/* iccProfile */
	if (image->icc.size > 0) {
		zend_update_property_stringl(codecs_info_ce, Z_OBJ_P(return_value), "iccProfile", sizeof("iccProfile") - 1, (char *)image->icc.data, image->icc.size);
	} else {
		zend_update_property_null(codecs_info_ce, Z_OBJ_P(return_value), "iccProfile", sizeof("iccProfile") - 1);
	}

	/* orientation */
	uint32_t orient_val = calc_exif_orientation(image->transformFlags, image->irot, image->imir);
	zend_object *orient_obj = NULL;
	if (zend_enum_get_case_by_value(&orient_obj, codecs_orientation_ce, orient_val, NULL, false) == SUCCESS && orient_obj) {
		zval zv;
		ZVAL_OBJ(&zv, orient_obj);
		zend_update_property(codecs_info_ce, Z_OBJ_P(return_value), "orientation", sizeof("orientation") - 1, &zv);
	}

	zend_update_property_long(codecs_info_ce, Z_OBJ_P(return_value), "frameCount", sizeof("frameCount") - 1, decoder->imageCount);
	zend_update_property_bool(codecs_info_ce, Z_OBJ_P(return_value), "animated", sizeof("animated") - 1, decoder->imageCount > 1);
	zend_update_property_long(codecs_info_ce, Z_OBJ_P(return_value), "byteSize", sizeof("byteSize") - 1, size);

	/* metadataBlocks array */
	zval blocks_zv;
	array_init(&blocks_zv);
	if (image->icc.size > 0) {
		add_assoc_stringl(&blocks_zv, "icc", (char *)image->icc.data, image->icc.size);
	}
	if (image->exif.size > 0) {
		add_assoc_stringl(&blocks_zv, "exif", (char *)image->exif.data, image->exif.size);
	}
	if (image->xmp.size > 0) {
		add_assoc_stringl(&blocks_zv, "xmp", (char *)image->xmp.data, image->xmp.size);
	}
	zend_update_property(codecs_info_ce, Z_OBJ_P(return_value), "metadataBlocks", sizeof("metadataBlocks") - 1, &blocks_zv);
	zval_ptr_dtor(&blocks_zv);

	avifDecoderDestroy(decoder);
	return true;
}

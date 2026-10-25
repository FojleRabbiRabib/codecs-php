/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2026 Fojle Rabbi (Rabib)
 *
 * codecs-php — constructors for readonly value objects and options.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "zend_exceptions.h"
#include "codecs_internal.h"

PHP_METHOD(Codecs_Cicp, __construct)
{
	zend_long colour_primaries;
	zend_long transfer_characteristics;
	zend_long matrix_coefficients;
	bool full_range;

	ZEND_PARSE_PARAMETERS_START(4, 4)
	Z_PARAM_LONG(colour_primaries)
	Z_PARAM_LONG(transfer_characteristics)
	Z_PARAM_LONG(matrix_coefficients)
	Z_PARAM_BOOL(full_range)
	ZEND_PARSE_PARAMETERS_END();
	(void)return_value;

	zend_update_property_long(codecs_cicp_ce, Z_OBJ_P(ZEND_THIS), "colourPrimaries", sizeof("colourPrimaries") - 1, colour_primaries);
	zend_update_property_long(codecs_cicp_ce, Z_OBJ_P(ZEND_THIS), "transferCharacteristics", sizeof("transferCharacteristics") - 1, transfer_characteristics);
	zend_update_property_long(codecs_cicp_ce, Z_OBJ_P(ZEND_THIS), "matrixCoefficients", sizeof("matrixCoefficients") - 1, matrix_coefficients);
	zend_update_property_bool(codecs_cicp_ce, Z_OBJ_P(ZEND_THIS), "fullRange", sizeof("fullRange") - 1, full_range);
}

PHP_METHOD(Codecs_DecodeOptions, __construct)
{
	zend_long max_dimension = 0;
	zend_long max_pixels = 0;
	bool ignore_icc = false;
	bool ignore_exif = false;
	bool ignore_xmp = false;
	bool keep_orientation = false;
	bool first_frame_only = false;
	zval *matrix_override = NULL;
	zval *threads = NULL;

	ZEND_PARSE_PARAMETERS_START(0, 9)
	Z_PARAM_OPTIONAL
	Z_PARAM_LONG(max_dimension)
	Z_PARAM_LONG(max_pixels)
	Z_PARAM_BOOL(ignore_icc)
	Z_PARAM_BOOL(ignore_exif)
	Z_PARAM_BOOL(ignore_xmp)
	Z_PARAM_BOOL(keep_orientation)
	Z_PARAM_BOOL(first_frame_only)
	Z_PARAM_OBJECT_OR_NULL(matrix_override)
	Z_PARAM_ZVAL(threads)
	ZEND_PARSE_PARAMETERS_END();
	(void)return_value;

	zend_update_property_long(codecs_decode_options_ce, Z_OBJ_P(ZEND_THIS), "maxDimension", sizeof("maxDimension") - 1, max_dimension);
	zend_update_property_long(codecs_decode_options_ce, Z_OBJ_P(ZEND_THIS), "maxPixels", sizeof("maxPixels") - 1, max_pixels);
	zend_update_property_bool(codecs_decode_options_ce, Z_OBJ_P(ZEND_THIS), "ignoreIcc", sizeof("ignoreIcc") - 1, ignore_icc);
	zend_update_property_bool(codecs_decode_options_ce, Z_OBJ_P(ZEND_THIS), "ignoreExif", sizeof("ignoreExif") - 1, ignore_exif);
	zend_update_property_bool(codecs_decode_options_ce, Z_OBJ_P(ZEND_THIS), "ignoreXmp", sizeof("ignoreXmp") - 1, ignore_xmp);
	zend_update_property_bool(codecs_decode_options_ce, Z_OBJ_P(ZEND_THIS), "keepOrientation", sizeof("keepOrientation") - 1, keep_orientation);
	zend_update_property_bool(codecs_decode_options_ce, Z_OBJ_P(ZEND_THIS), "firstFrameOnly", sizeof("firstFrameOnly") - 1, first_frame_only);

	if (matrix_override && Z_TYPE_P(matrix_override) == IS_OBJECT) {
		zend_update_property(codecs_decode_options_ce, Z_OBJ_P(ZEND_THIS), "matrixOverride", sizeof("matrixOverride") - 1, matrix_override);
	} else {
		zend_update_property_null(codecs_decode_options_ce, Z_OBJ_P(ZEND_THIS), "matrixOverride", sizeof("matrixOverride") - 1);
	}

	if (threads && Z_TYPE_P(threads) == IS_LONG) {
		zend_update_property_long(codecs_decode_options_ce, Z_OBJ_P(ZEND_THIS), "threads", sizeof("threads") - 1, Z_LVAL_P(threads));
	} else {
		zend_update_property_null(codecs_decode_options_ce, Z_OBJ_P(ZEND_THIS), "threads", sizeof("threads") - 1);
	}
}

PHP_METHOD(Codecs_AvifEncodeOptions, __construct)
{
	zend_long quality = 60;
	zval *quality_alpha = NULL;
	zend_long speed = 6;
	zval *subsampling = NULL;
	zval *matrix = NULL;
	bool lossless = false;
	zval *alpha = NULL;
	zval *metadata = NULL;
	zval *threads = NULL;

	ZEND_PARSE_PARAMETERS_START(0, 9)
	Z_PARAM_OPTIONAL
	Z_PARAM_LONG(quality)
	Z_PARAM_ZVAL(quality_alpha)
	Z_PARAM_LONG(speed)
	Z_PARAM_OBJECT(subsampling)
	Z_PARAM_OBJECT(matrix)
	Z_PARAM_BOOL(lossless)
	Z_PARAM_OBJECT(alpha)
	Z_PARAM_OBJECT(metadata)
	Z_PARAM_ZVAL(threads)
	ZEND_PARSE_PARAMETERS_END();
	(void)return_value;

	zend_update_property_long(codecs_avif_encode_options_ce, Z_OBJ_P(ZEND_THIS), "quality", sizeof("quality") - 1, quality);
	if (quality_alpha && Z_TYPE_P(quality_alpha) == IS_LONG) {
		zend_update_property_long(codecs_avif_encode_options_ce, Z_OBJ_P(ZEND_THIS), "qualityAlpha", sizeof("qualityAlpha") - 1, Z_LVAL_P(quality_alpha));
	} else {
		zend_update_property_null(codecs_avif_encode_options_ce, Z_OBJ_P(ZEND_THIS), "qualityAlpha", sizeof("qualityAlpha") - 1);
	}

	zend_update_property_long(codecs_avif_encode_options_ce, Z_OBJ_P(ZEND_THIS), "speed", sizeof("speed") - 1, speed);
	if (subsampling) {
		zend_update_property(codecs_avif_encode_options_ce, Z_OBJ_P(ZEND_THIS), "subsampling", sizeof("subsampling") - 1, subsampling);
	} else {
		zend_object *case_obj = zend_enum_get_case_cstr(codecs_chroma_subsampling_ce, "Yuv420");
		if (case_obj) {
			zval zv;
			ZVAL_OBJ(&zv, case_obj);
			zend_update_property(codecs_avif_encode_options_ce, Z_OBJ_P(ZEND_THIS), "subsampling", sizeof("subsampling") - 1, &zv);
		}
	}
	if (matrix) {
		zend_update_property(codecs_avif_encode_options_ce, Z_OBJ_P(ZEND_THIS), "matrix", sizeof("matrix") - 1, matrix);
	} else {
		zend_object *case_obj = zend_enum_get_case_cstr(codecs_matrix_coefficients_ce, "Bt601");
		if (case_obj) {
			zval zv;
			ZVAL_OBJ(&zv, case_obj);
			zend_update_property(codecs_avif_encode_options_ce, Z_OBJ_P(ZEND_THIS), "matrix", sizeof("matrix") - 1, &zv);
		}
	}
	zend_update_property_bool(codecs_avif_encode_options_ce, Z_OBJ_P(ZEND_THIS), "lossless", sizeof("lossless") - 1, lossless);
	if (alpha) {
		zend_update_property(codecs_avif_encode_options_ce, Z_OBJ_P(ZEND_THIS), "alpha", sizeof("alpha") - 1, alpha);
	} else {
		zend_object *case_obj = zend_enum_get_case_cstr(codecs_alpha_mode_ce, "Straight");
		if (case_obj) {
			zval zv;
			ZVAL_OBJ(&zv, case_obj);
			zend_update_property(codecs_avif_encode_options_ce, Z_OBJ_P(ZEND_THIS), "alpha", sizeof("alpha") - 1, &zv);
		}
	}
	if (metadata) {
		zend_update_property(codecs_avif_encode_options_ce, Z_OBJ_P(ZEND_THIS), "metadata", sizeof("metadata") - 1, metadata);
	} else {
		zend_object *case_obj = zend_enum_get_case_cstr(codecs_metadata_policy_ce, "Preserve");
		if (case_obj) {
			zval zv;
			ZVAL_OBJ(&zv, case_obj);
			zend_update_property(codecs_avif_encode_options_ce, Z_OBJ_P(ZEND_THIS), "metadata", sizeof("metadata") - 1, &zv);
		}
	}
	if (threads && Z_TYPE_P(threads) == IS_LONG) {
		zend_update_property_long(codecs_avif_encode_options_ce, Z_OBJ_P(ZEND_THIS), "threads", sizeof("threads") - 1, Z_LVAL_P(threads));
	} else {
		zend_update_property_null(codecs_avif_encode_options_ce, Z_OBJ_P(ZEND_THIS), "threads", sizeof("threads") - 1);
	}
}

PHP_METHOD(Codecs_JxlEncodeOptions, __construct)
{
	zend_long quality = 90;
	zend_long effort = 7;
	zend_long decoding_speed = 0;
	bool lossless = false;
	zval *alpha = NULL;
	zval *metadata = NULL;
	zval *threads = NULL;

	ZEND_PARSE_PARAMETERS_START(0, 7)
	Z_PARAM_OPTIONAL
	Z_PARAM_LONG(quality)
	Z_PARAM_LONG(effort)
	Z_PARAM_LONG(decoding_speed)
	Z_PARAM_BOOL(lossless)
	Z_PARAM_OBJECT(alpha)
	Z_PARAM_OBJECT(metadata)
	Z_PARAM_ZVAL(threads)
	ZEND_PARSE_PARAMETERS_END();
	(void)return_value;

	zend_update_property_long(codecs_jxl_encode_options_ce, Z_OBJ_P(ZEND_THIS), "quality", sizeof("quality") - 1, quality);
	zend_update_property_long(codecs_jxl_encode_options_ce, Z_OBJ_P(ZEND_THIS), "effort", sizeof("effort") - 1, effort);
	zend_update_property_long(codecs_jxl_encode_options_ce, Z_OBJ_P(ZEND_THIS), "decodingSpeed", sizeof("decodingSpeed") - 1, decoding_speed);
	zend_update_property_bool(codecs_jxl_encode_options_ce, Z_OBJ_P(ZEND_THIS), "lossless", sizeof("lossless") - 1, lossless);
	if (alpha) {
		zend_update_property(codecs_jxl_encode_options_ce, Z_OBJ_P(ZEND_THIS), "alpha", sizeof("alpha") - 1, alpha);
	} else {
		zend_object *case_obj = zend_enum_get_case_cstr(codecs_alpha_mode_ce, "Straight");
		if (case_obj) {
			zval zv;
			ZVAL_OBJ(&zv, case_obj);
			zend_update_property(codecs_jxl_encode_options_ce, Z_OBJ_P(ZEND_THIS), "alpha", sizeof("alpha") - 1, &zv);
		}
	}
	if (metadata) {
		zend_update_property(codecs_jxl_encode_options_ce, Z_OBJ_P(ZEND_THIS), "metadata", sizeof("metadata") - 1, metadata);
	} else {
		zend_object *case_obj = zend_enum_get_case_cstr(codecs_metadata_policy_ce, "Preserve");
		if (case_obj) {
			zval zv;
			ZVAL_OBJ(&zv, case_obj);
			zend_update_property(codecs_jxl_encode_options_ce, Z_OBJ_P(ZEND_THIS), "metadata", sizeof("metadata") - 1, &zv);
		}
	}
	if (threads && Z_TYPE_P(threads) == IS_LONG) {
		zend_update_property_long(codecs_jxl_encode_options_ce, Z_OBJ_P(ZEND_THIS), "threads", sizeof("threads") - 1, Z_LVAL_P(threads));
	} else {
		zend_update_property_null(codecs_jxl_encode_options_ce, Z_OBJ_P(ZEND_THIS), "threads", sizeof("threads") - 1);
	}
}

PHP_METHOD(Codecs_Info, __construct)
{
	zval *format;
	zend_long width, height, bit_depth;
	bool alpha;
	zval *cicp;
	zend_string *icc_profile = NULL;
	zval *orientation;
	zend_long frame_count;
	bool animated;
	zend_long byte_size;
	zval *metadata_blocks;

	ZEND_PARSE_PARAMETERS_START(12, 12)
	Z_PARAM_OBJECT(format)
	Z_PARAM_LONG(width)
	Z_PARAM_LONG(height)
	Z_PARAM_LONG(bit_depth)
	Z_PARAM_BOOL(alpha)
	Z_PARAM_OBJECT(cicp)
	Z_PARAM_STR_OR_NULL(icc_profile)
	Z_PARAM_OBJECT(orientation)
	Z_PARAM_LONG(frame_count)
	Z_PARAM_BOOL(animated)
	Z_PARAM_LONG(byte_size)
	Z_PARAM_ARRAY(metadata_blocks)
	ZEND_PARSE_PARAMETERS_END();
	(void)return_value;

	zend_update_property(codecs_info_ce, Z_OBJ_P(ZEND_THIS), "format", sizeof("format") - 1, format);
	zend_update_property_long(codecs_info_ce, Z_OBJ_P(ZEND_THIS), "width", sizeof("width") - 1, width);
	zend_update_property_long(codecs_info_ce, Z_OBJ_P(ZEND_THIS), "height", sizeof("height") - 1, height);
	zend_update_property_long(codecs_info_ce, Z_OBJ_P(ZEND_THIS), "bitDepth", sizeof("bitDepth") - 1, bit_depth);
	zend_update_property_bool(codecs_info_ce, Z_OBJ_P(ZEND_THIS), "alpha", sizeof("alpha") - 1, alpha);
	zend_update_property(codecs_info_ce, Z_OBJ_P(ZEND_THIS), "cicp", sizeof("cicp") - 1, cicp);

	if (icc_profile) {
		zend_update_property_str(codecs_info_ce, Z_OBJ_P(ZEND_THIS), "iccProfile", sizeof("iccProfile") - 1, icc_profile);
	} else {
		zend_update_property_null(codecs_info_ce, Z_OBJ_P(ZEND_THIS), "iccProfile", sizeof("iccProfile") - 1);
	}

	zend_update_property(codecs_info_ce, Z_OBJ_P(ZEND_THIS), "orientation", sizeof("orientation") - 1, orientation);
	zend_update_property_long(codecs_info_ce, Z_OBJ_P(ZEND_THIS), "frameCount", sizeof("frameCount") - 1, frame_count);
	zend_update_property_bool(codecs_info_ce, Z_OBJ_P(ZEND_THIS), "animated", sizeof("animated") - 1, animated);
	zend_update_property_long(codecs_info_ce, Z_OBJ_P(ZEND_THIS), "byteSize", sizeof("byteSize") - 1, byte_size);
	zend_update_property(codecs_info_ce, Z_OBJ_P(ZEND_THIS), "metadataBlocks", sizeof("metadataBlocks") - 1, metadata_blocks);
}

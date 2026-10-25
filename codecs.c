/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2026 Fojle Rabbi (Rabib)
 *
 * codecs-php — module lifecycle, class registration, and extension info.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/spl/spl_exceptions.h"
#include "ext/standard/info.h"
#include "avif/avif.h"
#include "dav1d/dav1d.h"
#include "php_codecs.h"
#include "codecs_internal.h"
#include "codecs_arginfo.h"

ZEND_DECLARE_MODULE_GLOBALS(codecs)

/* Registered class entries */
zend_class_entry *codecs_format_ce = NULL;
zend_class_entry *codecs_alpha_mode_ce = NULL;
zend_class_entry *codecs_chroma_subsampling_ce = NULL;
zend_class_entry *codecs_matrix_coefficients_ce = NULL;
zend_class_entry *codecs_orientation_ce = NULL;
zend_class_entry *codecs_metadata_policy_ce = NULL;
zend_class_entry *codecs_error_code_ce = NULL;

zend_class_entry *codecs_exception_ce = NULL;
zend_class_entry *codecs_decode_exception_ce = NULL;
zend_class_entry *codecs_encode_exception_ce = NULL;
zend_class_entry *codecs_limit_exception_ce = NULL;
zend_class_entry *codecs_unsupported_format_exception_ce = NULL;
zend_class_entry *codecs_corrupt_data_exception_ce = NULL;
zend_class_entry *codecs_stream_exception_ce = NULL;

zend_class_entry *codecs_encode_options_ce = NULL;
zend_class_entry *codecs_cicp_ce = NULL;
zend_class_entry *codecs_decode_options_ce = NULL;
zend_class_entry *codecs_avif_encode_options_ce = NULL;
zend_class_entry *codecs_jxl_encode_options_ce = NULL;
zend_class_entry *codecs_info_ce = NULL;
zend_class_entry *codecs_image_ce = NULL;
zend_class_entry *codecs_codec_ce = NULL;

extern void codecs_image_register_handlers(void);

PHP_INI_BEGIN()
STD_PHP_INI_ENTRY("codecs.max_dimension", "16384", PHP_INI_SYSTEM, OnUpdateLong, max_dimension, zend_codecs_globals, codecs_globals)
STD_PHP_INI_ENTRY("codecs.max_pixels", "16777216", PHP_INI_SYSTEM, OnUpdateLong, max_pixels, zend_codecs_globals, codecs_globals)
STD_PHP_INI_ENTRY("codecs.max_input_bytes", "268435456", PHP_INI_SYSTEM, OnUpdateLong, max_input_bytes, zend_codecs_globals, codecs_globals)
STD_PHP_INI_ENTRY("codecs.max_metadata_bytes", "8388608", PHP_INI_SYSTEM, OnUpdateLong, max_metadata_bytes, zend_codecs_globals, codecs_globals)
STD_PHP_INI_ENTRY("codecs.max_frames", "1", PHP_INI_SYSTEM, OnUpdateLong, max_frames, zend_codecs_globals, codecs_globals)
STD_PHP_INI_ENTRY("codecs.threads", "1", PHP_INI_ALL, OnUpdateLong, threads, zend_codecs_globals, codecs_globals)
STD_PHP_INI_ENTRY("codecs.memory_budget", "1073741824", PHP_INI_SYSTEM, OnUpdateLong, memory_budget, zend_codecs_globals, codecs_globals)
PHP_INI_END()

static void php_codecs_init_globals(zend_codecs_globals *codecs_globals)
{
	codecs_globals->max_dimension = CODECS_DEFAULT_MAX_DIMENSION;
	codecs_globals->max_pixels = CODECS_DEFAULT_MAX_PIXELS;
	codecs_globals->max_input_bytes = CODECS_DEFAULT_MAX_INPUT_BYTES;
	codecs_globals->max_metadata_bytes = CODECS_DEFAULT_MAX_METADATA_BYTES;
	codecs_globals->max_frames = CODECS_DEFAULT_MAX_FRAMES;
	codecs_globals->threads = 1;
	codecs_globals->memory_budget = CODECS_DEFAULT_MEMORY_BUDGET;
}

/* --- Codecs\Codec methods ------------------------------------------------ */

ZEND_METHOD(Codecs_Codec, version)
{
	ZEND_PARSE_PARAMETERS_NONE();
	RETURN_STRING(PHP_CODECS_VERSION);
}

ZEND_METHOD(Codecs_Codec, versions)
{
	ZEND_PARSE_PARAMETERS_NONE();

	array_init(return_value);
	add_assoc_string(return_value, "codecs_php", PHP_CODECS_VERSION);
	add_assoc_string(return_value, "libavif", avifVersion());
	add_assoc_string(return_value, "dav1d", dav1d_version());
	add_assoc_string(return_value, "aom", "not_compiled");
	add_assoc_string(return_value, "libjxl", "not_compiled");
	add_assoc_string(return_value, "highway", "not_compiled");
	add_assoc_string(return_value, "libpng", "not_compiled");
	add_assoc_string(return_value, "zlib", "not_compiled");
}

ZEND_METHOD(Codecs_Codec, supports)
{
	zval *format_zv;

	ZEND_PARSE_PARAMETERS_START(1, 1)
	Z_PARAM_OBJECT(format_zv)
	ZEND_PARSE_PARAMETERS_END();

	zval *case_val = zend_enum_fetch_case_name(Z_OBJ_P(format_zv));
	if (case_val && strcmp(Z_STRVAL_P(case_val), "Avif") == 0) {
		RETURN_TRUE;
	}
	RETURN_FALSE;
}

ZEND_METHOD(Codecs_Codec, decode)
{
	zend_string *data;
	zval *options_zv = NULL;

	ZEND_PARSE_PARAMETERS_START(1, 2)
	Z_PARAM_STR(data)
	Z_PARAM_OPTIONAL
	Z_PARAM_OBJECT_OR_NULL(options_zv)
	ZEND_PARSE_PARAMETERS_END();

	codecs_image_object *img = codecs_avif_decode((const uint8_t *)ZSTR_VAL(data), ZSTR_LEN(data), options_zv);
	if (!img) {
		RETURN_THROWS();
	}

	RETURN_OBJ(&img->std);
}

ZEND_METHOD(Codecs_Codec, decodeFile)
{
	zend_string *path;
	zval *options_zv = NULL;

	ZEND_PARSE_PARAMETERS_START(1, 2)
	Z_PARAM_PATH_STR(path)
	Z_PARAM_OPTIONAL
	Z_PARAM_OBJECT_OR_NULL(options_zv)
	ZEND_PARSE_PARAMETERS_END();

	php_stream *stream = php_stream_open_wrapper(ZSTR_VAL(path), "rb", REPORT_ERRORS, NULL);
	if (!stream) {
		RETURN_THROWS();
	}

	size_t max_bytes = (size_t)CODECS_G(max_input_bytes);
	zend_string *contents = php_stream_copy_to_mem(stream, max_bytes + 1, 0);

	if (!contents) {
		bool is_eof = php_stream_eof(stream);
		php_stream_close(stream);
		if (is_eof) {
			codecs_throw(codecs_corrupt_data_exception_ce, CODECS_ERR_BITSTREAM, "Cannot decode empty or zero-byte file");
		} else {
			codecs_throw(codecs_stream_exception_ce, CODECS_ERR_STREAM_IO, "Failed to read file: %s", ZSTR_VAL(path));
		}
		RETURN_THROWS();
	}
	php_stream_close(stream);

	if (ZSTR_LEN(contents) > max_bytes) {
		zend_string_release(contents);
		codecs_throw(codecs_limit_exception_ce, CODECS_ERR_LIMIT, "Input file exceeds maximum allowed input size (%zu bytes)", max_bytes);
		RETURN_THROWS();
	}

	codecs_image_object *img = codecs_avif_decode((const uint8_t *)ZSTR_VAL(contents), ZSTR_LEN(contents), options_zv);
	zend_string_release(contents);

	if (!img) {
		RETURN_THROWS();
	}

	RETURN_OBJ(&img->std);
}

ZEND_METHOD(Codecs_Codec, decodeStream)
{
	zval *zstream;
	zval *options_zv = NULL;

	ZEND_PARSE_PARAMETERS_START(1, 2)
	Z_PARAM_RESOURCE(zstream)
	Z_PARAM_OPTIONAL
	Z_PARAM_OBJECT_OR_NULL(options_zv)
	ZEND_PARSE_PARAMETERS_END();

	php_stream *stream;
	php_stream_from_zval(stream, zstream);

	size_t max_bytes = (size_t)CODECS_G(max_input_bytes);
	zend_string *contents = php_stream_copy_to_mem(stream, max_bytes + 1, 0);
	if (!contents) {
		if (php_stream_eof(stream)) {
			codecs_throw(codecs_corrupt_data_exception_ce, CODECS_ERR_BITSTREAM, "Cannot decode empty or zero-byte stream");
		} else {
			codecs_throw(codecs_stream_exception_ce, CODECS_ERR_STREAM_IO, "Failed to read stream to memory");
		}
		RETURN_THROWS();
	}

	if (ZSTR_LEN(contents) > max_bytes) {
		zend_string_release(contents);
		codecs_throw(codecs_limit_exception_ce, CODECS_ERR_LIMIT, "Input stream exceeds maximum allowed input size (%zu bytes)", max_bytes);
		RETURN_THROWS();
	}

	codecs_image_object *img = codecs_avif_decode((const uint8_t *)ZSTR_VAL(contents), ZSTR_LEN(contents), options_zv);
	zend_string_release(contents);

	if (!img) {
		RETURN_THROWS();
	}

	RETURN_OBJ(&img->std);
}

ZEND_METHOD(Codecs_Codec, encode)
{
	zval *image_zv;
	zval *format_zv;
	zval *options_zv = NULL;

	ZEND_PARSE_PARAMETERS_START(2, 3)
	Z_PARAM_OBJECT(image_zv)
	Z_PARAM_OBJECT(format_zv)
	Z_PARAM_OPTIONAL
	Z_PARAM_OBJECT_OR_NULL(options_zv)
	ZEND_PARSE_PARAMETERS_END();

	(void)image_zv;
	(void)format_zv;
	(void)options_zv;
	(void)return_value;
	codecs_throw(codecs_unsupported_format_exception_ce, CODECS_ERR_UNSUPPORTED,
				 "encode() is not yet implemented in this milestone");
}

ZEND_METHOD(Codecs_Codec, encodeFile)
{
	zval *image_zv;
	zend_string *path;
	zval *format_zv;
	zval *options_zv = NULL;

	ZEND_PARSE_PARAMETERS_START(3, 4)
	Z_PARAM_OBJECT(image_zv)
	Z_PARAM_PATH_STR(path)
	Z_PARAM_OBJECT(format_zv)
	Z_PARAM_OPTIONAL
	Z_PARAM_OBJECT_OR_NULL(options_zv)
	ZEND_PARSE_PARAMETERS_END();

	(void)image_zv;
	(void)path;
	(void)format_zv;
	(void)options_zv;
	(void)return_value;
	codecs_throw(codecs_unsupported_format_exception_ce, CODECS_ERR_UNSUPPORTED,
				 "encodeFile() is not yet implemented in this milestone");
}

ZEND_METHOD(Codecs_Codec, encodeStream)
{
	zval *image_zv;
	zval *zstream;
	zval *format_zv;
	zval *options_zv = NULL;

	ZEND_PARSE_PARAMETERS_START(3, 4)
	Z_PARAM_OBJECT(image_zv)
	Z_PARAM_RESOURCE(zstream)
	Z_PARAM_OBJECT(format_zv)
	Z_PARAM_OPTIONAL
	Z_PARAM_OBJECT_OR_NULL(options_zv)
	ZEND_PARSE_PARAMETERS_END();

	(void)image_zv;
	(void)zstream;
	(void)format_zv;
	(void)options_zv;
	(void)return_value;
	codecs_throw(codecs_unsupported_format_exception_ce, CODECS_ERR_UNSUPPORTED,
				 "encodeStream() is not yet implemented in this milestone");
}

ZEND_METHOD(Codecs_Codec, transcode)
{
	zend_string *data;
	zval *format_zv;
	zval *options_zv = NULL;

	ZEND_PARSE_PARAMETERS_START(2, 3)
	Z_PARAM_STR(data)
	Z_PARAM_OBJECT(format_zv)
	Z_PARAM_OPTIONAL
	Z_PARAM_OBJECT_OR_NULL(options_zv)
	ZEND_PARSE_PARAMETERS_END();

	(void)data;
	(void)format_zv;
	(void)options_zv;
	(void)return_value;
	codecs_throw(codecs_unsupported_format_exception_ce, CODECS_ERR_UNSUPPORTED,
				 "transcode() is not yet implemented in this milestone");
}

ZEND_METHOD(Codecs_Codec, probe)
{
	zend_string *data;

	ZEND_PARSE_PARAMETERS_START(1, 1)
	Z_PARAM_STR(data)
	ZEND_PARSE_PARAMETERS_END();

	if (!codecs_avif_probe((const uint8_t *)ZSTR_VAL(data), ZSTR_LEN(data), return_value)) {
		RETURN_THROWS();
	}
}

ZEND_METHOD(Codecs_Codec, probeFile)
{
	zend_string *path;

	ZEND_PARSE_PARAMETERS_START(1, 1)
	Z_PARAM_PATH_STR(path)
	ZEND_PARSE_PARAMETERS_END();

	php_stream *stream = php_stream_open_wrapper(ZSTR_VAL(path), "rb", REPORT_ERRORS, NULL);
	if (!stream) {
		RETURN_THROWS();
	}

	size_t max_bytes = (size_t)CODECS_G(max_input_bytes);
	zend_string *contents = php_stream_copy_to_mem(stream, max_bytes + 1, 0);

	if (!contents) {
		bool is_eof = php_stream_eof(stream);
		php_stream_close(stream);
		if (is_eof) {
			codecs_throw(codecs_corrupt_data_exception_ce, CODECS_ERR_BITSTREAM, "Cannot probe empty or zero-byte file");
		} else {
			codecs_throw(codecs_stream_exception_ce, CODECS_ERR_STREAM_IO, "Failed to read file: %s", ZSTR_VAL(path));
		}
		RETURN_THROWS();
	}
	php_stream_close(stream);

	if (ZSTR_LEN(contents) > max_bytes) {
		zend_string_release(contents);
		codecs_throw(codecs_limit_exception_ce, CODECS_ERR_LIMIT, "Input file exceeds maximum allowed input size (%zu bytes)", max_bytes);
		RETURN_THROWS();
	}

	bool ok = codecs_avif_probe((const uint8_t *)ZSTR_VAL(contents), ZSTR_LEN(contents), return_value);
	zend_string_release(contents);

	if (!ok) {
		RETURN_THROWS();
	}
}

ZEND_METHOD(Codecs_Codec, probeStream)
{
	zval *zstream;

	ZEND_PARSE_PARAMETERS_START(1, 1)
	Z_PARAM_RESOURCE(zstream)
	ZEND_PARSE_PARAMETERS_END();

	php_stream *stream;
	php_stream_from_zval(stream, zstream);

	size_t max_bytes = (size_t)CODECS_G(max_input_bytes);
	zend_string *contents = php_stream_copy_to_mem(stream, max_bytes + 1, 0);

	if (!contents) {
		if (php_stream_eof(stream)) {
			codecs_throw(codecs_corrupt_data_exception_ce, CODECS_ERR_BITSTREAM, "Cannot probe empty or zero-byte stream");
		} else {
			codecs_throw(codecs_stream_exception_ce, CODECS_ERR_STREAM_IO, "Failed to read stream to memory");
		}
		RETURN_THROWS();
	}

	if (ZSTR_LEN(contents) > max_bytes) {
		zend_string_release(contents);
		codecs_throw(codecs_limit_exception_ce, CODECS_ERR_LIMIT, "Input stream exceeds maximum allowed input size (%zu bytes)", max_bytes);
		RETURN_THROWS();
	}

	bool ok = codecs_avif_probe((const uint8_t *)ZSTR_VAL(contents), ZSTR_LEN(contents), return_value);
	zend_string_release(contents);

	if (!ok) {
		RETURN_THROWS();
	}
}

/* --- MINIT / MSHUTDOWN / MINFO ------------------------------------------- */

PHP_MINIT_FUNCTION(codecs)
{
	(void)type;

	ZEND_INIT_MODULE_GLOBALS(codecs, php_codecs_init_globals, NULL);
	REGISTER_INI_ENTRIES();

	/* Enums */
	codecs_format_ce = register_class_Codecs_Format();
	codecs_alpha_mode_ce = register_class_Codecs_AlphaMode();
	codecs_chroma_subsampling_ce = register_class_Codecs_ChromaSubsampling();
	codecs_matrix_coefficients_ce = register_class_Codecs_MatrixCoefficients();
	codecs_orientation_ce = register_class_Codecs_Orientation();
	codecs_metadata_policy_ce = register_class_Codecs_MetadataPolicy();
	codecs_error_code_ce = register_class_Codecs_ErrorCode();

	/* Exception hierarchy */
	codecs_exception_ce = register_class_Codecs_Exception(spl_ce_RuntimeException);
	codecs_decode_exception_ce = register_class_Codecs_DecodeException(codecs_exception_ce);
	codecs_encode_exception_ce = register_class_Codecs_EncodeException(codecs_exception_ce);
	codecs_limit_exception_ce = register_class_Codecs_LimitException(codecs_exception_ce);
	codecs_unsupported_format_exception_ce = register_class_Codecs_UnsupportedFormatException(codecs_exception_ce);
	codecs_corrupt_data_exception_ce = register_class_Codecs_CorruptDataException(codecs_exception_ce);
	codecs_stream_exception_ce = register_class_Codecs_StreamException(codecs_exception_ce);

	/* Options and supporting types */
	codecs_encode_options_ce = register_class_Codecs_EncodeOptions();
	codecs_cicp_ce = register_class_Codecs_Cicp();
	codecs_decode_options_ce = register_class_Codecs_DecodeOptions();
	codecs_avif_encode_options_ce = register_class_Codecs_AvifEncodeOptions(codecs_encode_options_ce);
	codecs_jxl_encode_options_ce = register_class_Codecs_JxlEncodeOptions(codecs_encode_options_ce);
	codecs_info_ce = register_class_Codecs_Info();

	/* Image class with custom object lifecycle */
	codecs_image_register_handlers();
	codecs_image_ce = register_class_Codecs_Image();
	codecs_image_ce->create_object = codecs_image_create;
	codecs_image_ce->ce_flags |= ZEND_ACC_NOT_SERIALIZABLE;

	/* Facade */
	codecs_codec_ce = register_class_Codecs_Codec();

	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(codecs)
{
	(void)type;
	(void)module_number;

	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}

PHP_MINFO_FUNCTION(codecs)
{
	(void)zend_module;

	php_info_print_table_start();
	php_info_print_table_header(2, "codecs support", "enabled");
	php_info_print_table_row(2, "Extension version", PHP_CODECS_VERSION);
	php_info_print_table_row(2, "libavif version", avifVersion());
	php_info_print_table_row(2, "dav1d version", dav1d_version());
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}

zend_module_entry codecs_module_entry = {
	STANDARD_MODULE_HEADER,
	PHP_CODECS_EXTNAME,
	NULL,
	PHP_MINIT(codecs),
	PHP_MSHUTDOWN(codecs),
	NULL,
	NULL,
	PHP_MINFO(codecs),
	PHP_CODECS_VERSION,
	STANDARD_MODULE_PROPERTIES};

#ifdef COMPILE_DL_CODECS
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(codecs)
#endif

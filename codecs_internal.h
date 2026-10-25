/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2026 Fojle Rabbi (Rabib)
 *
 * codecs-php — internal declarations, status codes, and resource ceilings.
 */

#ifndef CODECS_INTERNAL_H
#define CODECS_INTERNAL_H

#include "php.h"
#include "zend_enum.h"
#include <stdint.h>
#include <stdbool.h>

/* Ceilings (from the design record §4) */
#define CODECS_DEFAULT_MAX_DIMENSION 16384
#define CODECS_DEFAULT_MAX_PIXELS (16ULL * 1024ULL * 1024ULL) /* 16 M px, covers 4032x3024 iPhone photo */
#define CODECS_DEFAULT_MAX_INPUT_BYTES (256ULL * 1024ULL * 1024ULL)
#define CODECS_DEFAULT_MAX_METADATA_BYTES (8ULL * 1024ULL * 1024ULL)
#define CODECS_DEFAULT_MEMORY_BUDGET (1024ULL * 1024ULL * 1024ULL)
#define CODECS_DEFAULT_MAX_FRAMES 1

/* Error codes matching Codecs\ErrorCode enum */
enum codecs_error_code {
	CODECS_ERR_OK = 0,
	CODECS_ERR_CONTAINER = 1,
	CODECS_ERR_BITSTREAM = 2,
	CODECS_ERR_METADATA = 3,
	CODECS_ERR_LIMIT = 4,
	CODECS_ERR_DIMENSION = 5,
	CODECS_ERR_PIXELS = 6,
	CODECS_ERR_UNSUPPORTED = 7,
	CODECS_ERR_HEIF_NOT_COMPILED = 8,
	CODECS_ERR_STREAM_IO = 9,
	CODECS_ERR_OUT_OF_MEMORY = 10,
	CODECS_ERR_INTERNAL = 11,
};

/* Module globals (ZTS-safe) */
ZEND_BEGIN_MODULE_GLOBALS(codecs)
zend_long max_dimension;
zend_long max_pixels;
zend_long max_input_bytes;
zend_long max_metadata_bytes;
zend_long max_frames;
zend_long threads;
zend_long memory_budget;
ZEND_END_MODULE_GLOBALS(codecs)

ZEND_EXTERN_MODULE_GLOBALS(codecs)

#define CODECS_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(codecs, v)

#if defined(ZTS) && defined(COMPILE_DL_CODECS)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

/* Registered class entries */
extern zend_class_entry *codecs_format_ce;
extern zend_class_entry *codecs_alpha_mode_ce;
extern zend_class_entry *codecs_chroma_subsampling_ce;
extern zend_class_entry *codecs_matrix_coefficients_ce;
extern zend_class_entry *codecs_orientation_ce;
extern zend_class_entry *codecs_metadata_policy_ce;
extern zend_class_entry *codecs_error_code_ce;

extern zend_class_entry *codecs_exception_ce;
extern zend_class_entry *codecs_decode_exception_ce;
extern zend_class_entry *codecs_encode_exception_ce;
extern zend_class_entry *codecs_limit_exception_ce;
extern zend_class_entry *codecs_unsupported_format_exception_ce;
extern zend_class_entry *codecs_corrupt_data_exception_ce;
extern zend_class_entry *codecs_stream_exception_ce;

extern zend_class_entry *codecs_encode_options_ce;
extern zend_class_entry *codecs_cicp_ce;
extern zend_class_entry *codecs_decode_options_ce;
extern zend_class_entry *codecs_avif_encode_options_ce;
extern zend_class_entry *codecs_jxl_encode_options_ce;
extern zend_class_entry *codecs_info_ce;
extern zend_class_entry *codecs_image_ce;
extern zend_class_entry *codecs_codec_ce;

/* Internal CICP representation */
typedef struct _codecs_cicp {
	uint32_t colour_primaries;
	uint32_t transfer_characteristics;
	uint32_t matrix_coefficients;
	bool full_range;
} codecs_cicp;

/* Native Image object layout (owns 16-bit RGBA buffer) */
typedef struct _codecs_image_object {
	uint16_t *pixels; /* interleaved RGBA, 16-bit per component, full-scale */
	uint32_t width;
	uint32_t height;
	uint32_t bit_depth;	 /* source depth: 8 | 10 | 12 | 16 */
	uint32_t alpha_mode; /* 0 = Straight, 1 = Premultiplied */
	char *provenance;	 /* "avif", "jxl", "png", "gd", "pixels" */
	codecs_cicp cicp;
	uint8_t *icc_profile;
	size_t icc_size;
	uint8_t *exif;
	size_t exif_size;
	uint8_t *xmp;
	size_t xmp_size;
	uint32_t orientation; /* EXIF 1..8 */
	bool closed;
	zend_object std;
} codecs_image_object;

static zend_always_inline codecs_image_object *codecs_image_from_obj(zend_object *obj)
{
	return (codecs_image_object *)((char *)(obj)-XtOffsetOf(codecs_image_object, std));
}

#define Z_CODECS_IMAGE_P(zv) codecs_image_from_obj(Z_OBJ_P(zv))

/* Memory and ceiling helper */
static zend_always_inline bool codecs_check_dimensions(uint32_t width, uint32_t height, int64_t max_dim, int64_t max_pix)
{
	if (width == 0 || height == 0) {
		return false;
	}
	if (max_dim > 0 && ((int64_t)width > max_dim || (int64_t)height > max_dim)) {
		return false;
	}
	uint64_t total_pixels = (uint64_t)width * (uint64_t)height;
	if (max_pix > 0 && total_pixels > (uint64_t)max_pix) {
		return false;
	}
	return true;
}

/* Throw helpers */
void codecs_throw(zend_class_entry *ce, enum codecs_error_code code, const char *format, ...);

/* Image object lifecycle */
zend_object *codecs_image_create(zend_class_entry *ce);
void codecs_image_free(zend_object *obj);
codecs_image_object *codecs_image_alloc(uint32_t width, uint32_t height, uint32_t bit_depth, uint32_t alpha_mode, const char *provenance);

/* AVIF decoder entry */
codecs_image_object *codecs_avif_decode(const uint8_t *data, size_t size, zval *options_zv);
bool codecs_avif_probe(const uint8_t *data, size_t size, zval *info_zv);

#endif /* CODECS_INTERNAL_H */

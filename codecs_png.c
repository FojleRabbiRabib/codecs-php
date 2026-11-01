/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2026 Fojle Rabbi (Rabib)
 *
 * codecs-php — PNG on-ramp (Image::fromPng and Image::toPng) using vendored libpng.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "zend_exceptions.h"
#include "codecs_internal.h"
#include "png.h"
#include <setjmp.h>
#include <string.h>

typedef struct {
	const uint8_t *input;
	size_t input_size;
	size_t input_offset;
	png_structp png;
	png_infop info;
	png_bytepp rows;
	uint16_t *pixels; /* decode RGBA16 buffer or temporary encode row buffer */
	uint8_t *icc;
	size_t icc_size;
	uint8_t *write_buffer;
	size_t write_capacity;
	size_t write_length;
	char error_message[192];
	char failure_message[256];
	bool metadata_limit;
	bool is_write;
	zend_class_entry *exception_ce;
	enum codecs_error_code error_code;
} codecs_png_context;

static void codecs_png_fail(codecs_png_context *ctx, zend_class_entry *ce, enum codecs_error_code code, const char *fmt, ...)
{
	va_list args;
	ctx->exception_ce = ce;
	ctx->error_code = code;
	va_start(args, fmt);
	vsnprintf(ctx->failure_message, sizeof(ctx->failure_message), fmt, args);
	va_end(args);
}

static void codecs_png_error_fn(png_structp png_ptr, png_const_charp error_msg)
{
	codecs_png_context *ctx = (codecs_png_context *)png_get_error_ptr(png_ptr);
	if (ctx && error_msg) {
		snprintf(ctx->error_message, sizeof(ctx->error_message), "%s", error_msg);
		if (strstr(error_msg, "profile too long") != NULL || strstr(error_msg, "too large") != NULL ||
			strstr(error_msg, "chunk data is too large") != NULL) {
			ctx->metadata_limit = true;
		}
	}
	png_longjmp(png_ptr, 1);
}

static void codecs_png_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
{
	(void)png_ptr;
	(void)warning_msg;
}

static void codecs_png_read_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
	codecs_png_context *ctx = (codecs_png_context *)png_get_io_ptr(png_ptr);
	if (!ctx || ctx->input_offset > ctx->input_size || length > ctx->input_size - ctx->input_offset) {
		png_error(png_ptr, "Unexpected end of PNG input stream");
		return;
	}
	memcpy(data, ctx->input + ctx->input_offset, length);
	ctx->input_offset += length;
}

static void codecs_png_write_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
	codecs_png_context *ctx = (codecs_png_context *)png_get_io_ptr(png_ptr);
	if (!ctx || length > SIZE_MAX - ctx->write_length) {
		png_error(png_ptr, "PNG output size overflow");
		return;
	}
	if (ctx->write_length + length > ctx->write_capacity) {
		size_t required = ctx->write_length + length;
		size_t new_capacity = ctx->write_capacity;
		while (new_capacity < required) {
			if (new_capacity > SIZE_MAX / 2) {
				new_capacity = required;
				break;
			}
			new_capacity *= 2;
		}
		ctx->write_buffer = erealloc(ctx->write_buffer, new_capacity);
		ctx->write_capacity = new_capacity;
	}
	memcpy(ctx->write_buffer + ctx->write_length, data, length);
	ctx->write_length += length;
}

static void codecs_png_flush_fn(png_structp png_ptr)
{
	(void)png_ptr;
}

static void codecs_png_context_cleanup(codecs_png_context *ctx)
{
	if (!ctx) {
		return;
	}
	if (ctx->png) {
		if (ctx->is_write) {
			png_destroy_write_struct(&ctx->png, ctx->info ? &ctx->info : NULL);
		} else {
			png_destroy_read_struct(&ctx->png, ctx->info ? &ctx->info : NULL, NULL);
		}
	}
	if (ctx->rows) {
		efree(ctx->rows);
		ctx->rows = NULL;
	}
	if (ctx->pixels) {
		efree(ctx->pixels);
		ctx->pixels = NULL;
	}
	if (ctx->icc) {
		efree(ctx->icc);
		ctx->icc = NULL;
	}
	if (ctx->write_buffer) {
		efree(ctx->write_buffer);
		ctx->write_buffer = NULL;
	}
}

static bool codecs_png_decode_core(codecs_png_context *ctx, const uint8_t *data, size_t size,
								   bool ignore_icc, int64_t max_dimension, int64_t max_pixels, uint32_t *width_out,
								   uint32_t *height_out, int *source_depth_out, codecs_cicp *cicp_out)
{
	ctx->input = data;
	ctx->input_size = size;
	ctx->input_offset = 8; /* PNG signature already checked by caller */
	ctx->png = png_create_read_struct(PNG_LIBPNG_VER_STRING, ctx, codecs_png_error_fn, codecs_png_warning_fn);
	if (!ctx->png) {
		codecs_png_fail(ctx, codecs_decode_exception_ce, CODECS_ERR_OUT_OF_MEMORY, "Failed to allocate PNG reader structure");
		return false;
	}
	ctx->info = png_create_info_struct(ctx->png);
	if (!ctx->info) {
		codecs_png_fail(ctx, codecs_decode_exception_ce, CODECS_ERR_OUT_OF_MEMORY, "Failed to allocate PNG info structure");
		return false;
	}

	/* Bound libpng's own chunk/profile allocations before it reads any chunks.
	 * png_icc_check_length rejects an oversized decompressed iCCP profile. */
	png_set_chunk_malloc_max(ctx->png, (png_alloc_size_t)CODECS_G(max_metadata_bytes));
	png_set_benign_errors(ctx->png, 0);

	/* setjmp state and error text live in the heap context, not modified automatic
	 * locals, so both libpng's longjmp and Zend bailout cleanup are well-defined. */
	if (setjmp(png_jmpbuf(ctx->png)) != 0) {
		if (ctx->metadata_limit) {
			codecs_png_fail(ctx, codecs_limit_exception_ce, CODECS_ERR_METADATA,
							"PNG metadata exceeds configured limit (%ld bytes): %s",
							CODECS_G(max_metadata_bytes), ctx->error_message);
		} else {
			codecs_png_fail(ctx, codecs_corrupt_data_exception_ce, CODECS_ERR_BITSTREAM,
							"PNG decoding error: %s", ctx->error_message[0] ? ctx->error_message : "malformed bitstream");
		}
		return false;
	}

	png_set_read_fn(ctx->png, ctx, codecs_png_read_fn);
	png_set_sig_bytes(ctx->png, 8);
	png_read_info(ctx->png, ctx->info);

	png_uint_32 width = 0, height = 0;
	int bit_depth = 0, color_type = -1, interlace_type = -1;
	png_get_IHDR(ctx->png, ctx->info, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);

	if (interlace_type != PNG_INTERLACE_NONE) {
		codecs_png_fail(ctx, codecs_unsupported_format_exception_ce, CODECS_ERR_UNSUPPORTED,
						"Interlaced PNG images are not supported in the supported subset");
		return false;
	}
	if (color_type != PNG_COLOR_TYPE_RGB && color_type != PNG_COLOR_TYPE_RGB_ALPHA && color_type != PNG_COLOR_TYPE_PALETTE) {
		codecs_png_fail(ctx, codecs_unsupported_format_exception_ce, CODECS_ERR_UNSUPPORTED,
						"Unsupported PNG color type (%d); only RGB, RGBA, and palette images are supported", color_type);
		return false;
	}
	if ((color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGB_ALPHA) && bit_depth != 8 && bit_depth != 16) {
		codecs_png_fail(ctx, codecs_unsupported_format_exception_ce, CODECS_ERR_UNSUPPORTED,
						"PNG truecolor bit depth (%d) must be 8 or 16", bit_depth);
		return false;
	}
	if (color_type == PNG_COLOR_TYPE_PALETTE && bit_depth != 1 && bit_depth != 2 && bit_depth != 4 && bit_depth != 8) {
		codecs_png_fail(ctx, codecs_unsupported_format_exception_ce, CODECS_ERR_UNSUPPORTED,
						"PNG palette bit depth (%d) must be 1, 2, 4, or 8", bit_depth);
		return false;
	}

	if (width == 0 || height == 0 || (int64_t)width > max_dimension || (int64_t)height > max_dimension) {
		codecs_png_fail(ctx, codecs_limit_exception_ce, CODECS_ERR_DIMENSION,
						"PNG dimensions (%ux%u) exceed max dimension limit (%ld)", width, height, max_dimension);
		return false;
	}
	uint64_t pixel_count = (uint64_t)width * (uint64_t)height;
	if (pixel_count > (uint64_t)max_pixels) {
		codecs_png_fail(ctx, codecs_limit_exception_ce, CODECS_ERR_PIXELS,
						"PNG pixel count (%llu) exceeds max pixels limit (%ld)", (unsigned long long)pixel_count, max_pixels);
		return false;
	}

	int source_depth = bit_depth;
	bool has_trns = png_get_valid(ctx->png, ctx->info, PNG_INFO_tRNS) != 0;
	if (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_palette_to_rgb(ctx->png);
	}
	if (has_trns) {
		png_set_tRNS_to_alpha(ctx->png);
	}
	if ((color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_PALETTE) && !has_trns) {
		png_set_filler(ctx->png, 0xFFFF, PNG_FILLER_AFTER);
	}
	if (bit_depth < 16) {
		png_set_expand_16(ctx->png);
	}
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	png_set_swap(ctx->png);
#endif
	png_read_update_info(ctx->png, ctx->info);

	/* The enforced subset must produce interleaved RGBA16 with no row padding. */
	size_t row_bytes = png_get_rowbytes(ctx->png, ctx->info);
	if (row_bytes != (size_t)width * 8 || (size_t)height > SIZE_MAX / row_bytes) {
		codecs_png_fail(ctx, codecs_unsupported_format_exception_ce, CODECS_ERR_UNSUPPORTED,
						"PNG transforms produced an unexpected row layout");
		return false;
	}
	size_t pixel_bytes = row_bytes * (size_t)height;
	/* height is bounded to max_dimension (16,384), so this table-size
	 * multiplication is safe on supported 32-bit and 64-bit targets. */
	size_t row_table_bytes = (size_t)height * sizeof(png_bytep);
	if (pixel_bytes > (uint64_t)CODECS_G(memory_budget) || row_table_bytes > (uint64_t)CODECS_G(memory_budget) - pixel_bytes) {
		codecs_png_fail(ctx, codecs_limit_exception_ce, CODECS_ERR_LIMIT,
						"PNG pixel and row buffers exceed memory budget (%ld bytes)", CODECS_G(memory_budget));
		return false;
	}

	ctx->pixels = emalloc(pixel_bytes);
	ctx->rows = emalloc(row_table_bytes);
	for (png_uint_32 y = 0; y < height; y++) {
		ctx->rows[y] = (png_bytep)((uint8_t *)ctx->pixels + (size_t)y * row_bytes);
	}
	png_read_image(ctx->png, ctx->rows);
	png_read_end(ctx->png, NULL);

	/* The chunk allocation cap above prevents oversized iCCP inflation before
	 * png_read_info returns. Check the exact decoded profile size again before
	 * copying it to extension-owned memory. */
	if (!ignore_icc) {
		png_charp icc_name = NULL;
		int compression_type = 0;
		png_bytep icc_data = NULL;
		png_uint_32 icc_len = 0;
		if (png_get_iCCP(ctx->png, ctx->info, &icc_name, &compression_type, &icc_data, &icc_len) == PNG_INFO_iCCP && icc_len > 0) {
			if (icc_len > (size_t)CODECS_G(max_metadata_bytes)) {
				ctx->metadata_limit = true;
				codecs_png_fail(ctx, codecs_limit_exception_ce, CODECS_ERR_METADATA,
								"ICC profile size (%u bytes) exceeds configured limit (%ld bytes)", icc_len, CODECS_G(max_metadata_bytes));
				return false;
			}
			size_t owned = pixel_bytes + row_table_bytes;
			if (owned > (size_t)CODECS_G(memory_budget) || (size_t)icc_len > (size_t)CODECS_G(memory_budget) - owned) {
				codecs_png_fail(ctx, codecs_limit_exception_ce, CODECS_ERR_LIMIT,
								"PNG pixel and ICC buffers exceed memory budget (%ld bytes)", CODECS_G(memory_budget));
				return false;
			}
			ctx->icc = emalloc(icc_len);
			memcpy(ctx->icc, icc_data, icc_len);
			ctx->icc_size = (size_t)icc_len;
		}
	}

	/* cICP has highest precedence; sRGB is used only when no cICP exists.
	 * gAMA/cHRM without an exact CICP mapping are not mislabeled as sRGB. */
	memset(cicp_out, 0, sizeof(*cicp_out));
	cicp_out->colour_primaries = 2;			/* unspecified */
	cicp_out->transfer_characteristics = 2; /* unspecified */
	cicp_out->matrix_coefficients = 0;		/* identity for RGB */
	cicp_out->full_range = true;
#ifdef PNG_cICP_SUPPORTED
	png_byte primaries = 0, transfer = 0, matrix = 0, full_range = 1;
	if (png_get_cICP(ctx->png, ctx->info, &primaries, &transfer, &matrix, &full_range) != 0) {
		cicp_out->colour_primaries = primaries;
		cicp_out->transfer_characteristics = transfer;
		cicp_out->matrix_coefficients = matrix;
		cicp_out->full_range = (full_range != 0);
	} else
#endif
	{
		int intent = 0;
		if (png_get_sRGB(ctx->png, ctx->info, &intent) == PNG_INFO_sRGB) {
			cicp_out->colour_primaries = 1;
			cicp_out->transfer_characteristics = 13;
			cicp_out->matrix_coefficients = 0;
			cicp_out->full_range = true;
		}
	}

	*width_out = width;
	*height_out = height;
	*source_depth_out = source_depth;
	return true;
}

codecs_image_object *codecs_png_decode(const uint8_t *data, size_t size, zval *options_zv)
{
	if (!data || size == 0) {
		codecs_throw(codecs_corrupt_data_exception_ce, CODECS_ERR_BITSTREAM, "Cannot decode empty or zero-byte PNG data");
		return NULL;
	}
	if (size > (size_t)CODECS_G(max_input_bytes)) {
		codecs_throw(codecs_limit_exception_ce, CODECS_ERR_LIMIT,
					 "Input data (%zu bytes) exceeds configured limit (%zu bytes)", size, (size_t)CODECS_G(max_input_bytes));
		return NULL;
	}
	static const uint8_t png_signature[8] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1a, '\n'};
	if (size < sizeof(png_signature) || memcmp(data, png_signature, sizeof(png_signature)) != 0) {
		codecs_throw(codecs_unsupported_format_exception_ce, CODECS_ERR_UNSUPPORTED, "Data does not have a valid PNG signature");
		return NULL;
	}

	bool ignore_icc = false;
	int64_t max_dimension = CODECS_G(max_dimension);
	int64_t max_pixels = CODECS_G(max_pixels);
	if (options_zv && Z_TYPE_P(options_zv) == IS_OBJECT) {
		zval rv;
		zval *prop = zend_read_property(codecs_decode_options_ce, Z_OBJ_P(options_zv), "ignoreIcc", sizeof("ignoreIcc") - 1, 1, &rv);
		ignore_icc = prop && Z_TYPE_P(prop) == IS_TRUE;
		prop = zend_read_property(codecs_decode_options_ce, Z_OBJ_P(options_zv), "maxDimension", sizeof("maxDimension") - 1, 1, &rv);
		if (prop && Z_TYPE_P(prop) == IS_LONG && Z_LVAL_P(prop) > 0 && Z_LVAL_P(prop) < max_dimension) {
			max_dimension = Z_LVAL_P(prop);
		}
		prop = zend_read_property(codecs_decode_options_ce, Z_OBJ_P(options_zv), "maxPixels", sizeof("maxPixels") - 1, 1, &rv);
		if (prop && Z_TYPE_P(prop) == IS_LONG && Z_LVAL_P(prop) > 0 && Z_LVAL_P(prop) < max_pixels) {
			max_pixels = Z_LVAL_P(prop);
		}
	}

	/* Context pointer is established before the Zend bailout guard and is never
	 * modified, so it remains valid if any emalloc/erealloc bails out. */
	codecs_png_context *ctx = ecalloc(1, sizeof(*ctx));
	uint32_t width = 0, height = 0;
	int source_depth = 8;
	codecs_cicp cicp;
	bool ok = false;
	zval image_zv;
	ZVAL_UNDEF(&image_zv);
	char failure[256] = "";
	zend_class_entry *failure_ce = NULL;
	enum codecs_error_code failure_code = CODECS_ERR_INTERNAL;

	zend_try
	{
		ok = codecs_png_decode_core(ctx, data, size, ignore_icc, max_dimension, max_pixels,
									&width, &height, &source_depth, &cicp);
		if (!ok) {
			failure_ce = ctx->exception_ce;
			failure_code = ctx->error_code;
			snprintf(failure, sizeof(failure), "%s", ctx->failure_message);
		} else {
			char *provenance = estrdup("png");
			object_init_ex(&image_zv, codecs_image_ce);
			codecs_image_object *img = Z_CODECS_IMAGE_P(&image_zv);
			img->pixels = ctx->pixels;
			img->width = width;
			img->height = height;
			img->bit_depth = (source_depth == 8) ? 8 : 16;
			img->alpha_mode = 0;
			img->provenance = provenance;
			img->orientation = 1;
			img->cicp = cicp;
			img->icc_profile = ctx->icc;
			img->icc_size = ctx->icc_size;
			ctx->pixels = NULL;
			ctx->icc = NULL;
		}
	}
	zend_catch
	{
		codecs_png_context_cleanup(ctx);
		efree(ctx);
		zend_bailout();
	}
	zend_end_try();

	codecs_png_context_cleanup(ctx);
	efree(ctx);
	if (!ok) {
		codecs_throw(failure_ce ? failure_ce : codecs_corrupt_data_exception_ce, failure_code,
					 "%s", failure[0] ? failure : "PNG decoding failed");
		return NULL;
	}
	return Z_CODECS_IMAGE_P(&image_zv);
}

static void codecs_png_write_error_fn(png_structp png_ptr, png_const_charp error_msg)
{
	codecs_png_context *ctx = (codecs_png_context *)png_get_error_ptr(png_ptr);
	if (ctx && error_msg) {
		snprintf(ctx->error_message, sizeof(ctx->error_message), "%s", error_msg);
	}
	png_longjmp(png_ptr, 1);
}

static bool codecs_png_encode_core(codecs_png_context *ctx, const codecs_image_object *img)
{
	ctx->is_write = true;
	ctx->write_capacity = 65536;
	ctx->write_length = 0;
	ctx->write_buffer = emalloc(ctx->write_capacity);
	ctx->error_message[0] = '\0';

	ctx->png = png_create_write_struct(PNG_LIBPNG_VER_STRING, ctx, codecs_png_write_error_fn, codecs_png_warning_fn);
	if (!ctx->png) {
		codecs_png_fail(ctx, codecs_encode_exception_ce, CODECS_ERR_OUT_OF_MEMORY, "Failed to allocate PNG writer structure");
		return false;
	}
	ctx->info = png_create_info_struct(ctx->png);
	if (!ctx->info) {
		codecs_png_fail(ctx, codecs_encode_exception_ce, CODECS_ERR_OUT_OF_MEMORY, "Failed to allocate PNG info structure");
		return false;
	}
	if (setjmp(png_jmpbuf(ctx->png)) != 0) {
		codecs_png_fail(ctx, codecs_encode_exception_ce, CODECS_ERR_BITSTREAM,
						"PNG encoding error: %s", ctx->error_message[0] ? ctx->error_message : "failed to write PNG");
		return false;
	}

	png_set_write_fn(ctx->png, ctx, codecs_png_write_fn, codecs_png_flush_fn);
	int out_depth = (img->bit_depth == 8) ? 8 : 16;
	png_set_IHDR(ctx->png, ctx->info, img->width, img->height, out_depth,
				 PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
				 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	if (img->icc_profile && img->icc_size > 0) {
		if (img->icc_size > (size_t)CODECS_G(max_metadata_bytes) || img->icc_size > PNG_UINT_31_MAX) {
			codecs_png_fail(ctx, codecs_limit_exception_ce, CODECS_ERR_METADATA, "Image ICC profile exceeds the configured PNG metadata limit");
			return false;
		}
		png_set_iCCP(ctx->png, ctx->info, "codecs", PNG_COMPRESSION_TYPE_BASE,
					 (png_const_bytep)img->icc_profile, (png_uint_32)img->icc_size);
	}
	png_write_info(ctx->png, ctx->info);

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	if (out_depth == 16) {
		png_set_swap(ctx->png);
	}
#endif

	size_t row_bytes = (size_t)img->width * (size_t)out_depth / 8 * 4;
	ctx->rows = emalloc((size_t)img->height * sizeof(png_bytep));
	if (out_depth == 16) {
		for (uint32_t y = 0; y < img->height; y++) {
			ctx->rows[y] = (png_bytep)((uint8_t *)img->pixels + (size_t)y * row_bytes);
		}
	} else {
		size_t tmp_size = row_bytes * (size_t)img->height;
		ctx->pixels = (uint16_t *)emalloc(tmp_size);
		uint8_t *tmp = (uint8_t *)ctx->pixels;
		const uint16_t *src = img->pixels;
		for (size_t i = 0; i < (size_t)img->width * img->height * 4; i++) {
			tmp[i] = (uint8_t)(src[i] >> 8);
		}
		for (uint32_t y = 0; y < img->height; y++) {
			ctx->rows[y] = tmp + (size_t)y * row_bytes;
		}
	}

	png_write_image(ctx->png, ctx->rows);
	png_write_end(ctx->png, ctx->info);
	return true;
}

zend_string *codecs_png_encode(const codecs_image_object *img)
{
	if (!img || !img->pixels || img->closed) {
		codecs_throw(codecs_encode_exception_ce, CODECS_ERR_INTERNAL, "Cannot encode null or closed Image to PNG");
		return NULL;
	}

	codecs_png_context *ctx = ecalloc(1, sizeof(*ctx));
	ctx->exception_ce = NULL;
	ctx->failure_message[0] = '\0';
	bool ok = false;
	zend_string *result = NULL;

	zend_try
	{
		ok = codecs_png_encode_core(ctx, img);
		if (ok) {
			result = zend_string_init((const char *)ctx->write_buffer, ctx->write_length, 0);
		}
	}
	zend_catch
	{
		codecs_png_context_cleanup(ctx);
		efree(ctx);
		zend_bailout();
	}
	zend_end_try();

	if (!ok) {
		char failure[256];
		snprintf(failure, sizeof(failure), "%s", ctx->failure_message[0] ? ctx->failure_message : "PNG encoding failed");
		zend_class_entry *ce = ctx->exception_ce ? ctx->exception_ce : codecs_encode_exception_ce;
		enum codecs_error_code code = ctx->error_code;
		codecs_png_context_cleanup(ctx);
		efree(ctx);
		codecs_throw(ce, code, "%s", failure);
		return NULL;
	}

	codecs_png_context_cleanup(ctx);
	efree(ctx);
	return result;
}

/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2026 Fojle Rabbi (Rabib)
 *
 * codecs-php — JPEG XL decoding using vendored libjxl.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "zend_exceptions.h"
#include "zend_enum.h"
#include "codecs_internal.h"
#include "jxl/decode.h"
#include "jxl/thread_parallel_runner.h"
#include "jxl/codestream_header.h"
#include "jxl/color_encoding.h"
#include <string.h>

/* Map a JxlColorEncoding to CICP numbers (JXL transfer enums match CICP codes;
 * primaries need a white-point split: SRGB with D65 to primaries 1, BT.2100 to
 * 9, P3 with DCI white to 11, P3 with D65 to 12, otherwise unknown (2)). */
static void codecs_jxl_cicp(const JxlColorEncoding *ce, codecs_cicp *out)
{
	switch (ce->primaries) {
	case JXL_PRIMARIES_SRGB:
		if (ce->white_point == JXL_WHITE_POINT_D65) {
			out->colour_primaries = 1;
		} else {
			out->colour_primaries = 2;
		}
		break;
	case JXL_PRIMARIES_2100:
		out->colour_primaries = 9;
		break;
	case JXL_PRIMARIES_P3:
		out->colour_primaries = (ce->white_point == JXL_WHITE_POINT_DCI) ? 11 : 12;
		break;
	default:
		out->colour_primaries = 2;
		break;
	}

	switch (ce->transfer_function) {
	case JXL_TRANSFER_FUNCTION_709:
		out->transfer_characteristics = 1;
		break;
	case JXL_TRANSFER_FUNCTION_LINEAR:
		out->transfer_characteristics = 8;
		break;
	case JXL_TRANSFER_FUNCTION_SRGB:
		out->transfer_characteristics = 13;
		break;
	case JXL_TRANSFER_FUNCTION_PQ:
		out->transfer_characteristics = 16;
		break;
	case JXL_TRANSFER_FUNCTION_DCI:
		out->transfer_characteristics = 17;
		break;
	case JXL_TRANSFER_FUNCTION_HLG:
		out->transfer_characteristics = 18;
		break;
	default:
		out->transfer_characteristics = 2;
		break;
	}

	/* Decoded RGB output is full range by construction. */
	out->matrix_coefficients = 0;
	out->full_range = true;
}

/* Fetch the ICC profile bytes at the COLOR_ENCODING stage. Returns 0 on
 * success; *icc is emalloc'd and owned by the caller. */
static int codecs_jxl_get_icc(JxlDecoder *dec, uint8_t **icc, size_t *icc_size)
{
	size_t size = 0;
	if (JxlDecoderGetICCProfileSize(dec, JXL_COLOR_PROFILE_TARGET_DATA, &size) != JXL_DEC_SUCCESS || size == 0) {
		return 1;
	}
	uint8_t *buf = emalloc(size);
	if (JxlDecoderGetColorAsICCProfile(dec, JXL_COLOR_PROFILE_TARGET_DATA, buf, size) != JXL_DEC_SUCCESS) {
		efree(buf);
		return 1;
	}
	*icc = buf;
	*icc_size = size;
	return 0;
}

/* Exif orientation reader for the JXL "Exif" box: locate the TIFF header via
 * the standard 4-byte offset prefix, then read tag 0x0112 from IFD0. */
static uint32_t codecs_jxl_exif_orientation(const uint8_t *exif, size_t size)
{
	if (size < 8) {
		return 1;
	}
	/* TIFF header offset: big-endian uint32 at bytes 0..3 ("MM\x00" style) */
	uint32_t tiff_off = ((uint32_t)exif[0] << 24) | ((uint32_t)exif[1] << 16) |
						((uint32_t)exif[2] << 8) | (uint32_t)exif[3];
	uint32_t le = 0;
	if (tiff_off == 0x00000006 || tiff_off == 0x00000008) {
		le = 1; /* standard offsets; the payload is a byte-order marker next */
	} else if (tiff_off > 0x10000000u) {
		le = 0;
	} else {
		le = 1;
	}
	(void)tiff_off;

	/* Some payloads carry the Exif\0\0 marker; skip six bytes when present. */
	size_t base = 0;
	if (size >= 6 && memcmp(exif, "Exif\0\0", 6) == 0) {
		base = 6;
	}
	if (size < base + 8) {
		return 1;
	}

	const uint8_t *tiff = exif + base;
	bool little;
	if (memcmp(tiff, "II", 2) == 0) {
		little = true;
	} else if (memcmp(tiff, "MM", 2) == 0) {
		little = false;
	} else {
		return 1;
	}
	(void)le;

	uint32_t ifd0 = little
						? (uint32_t)tiff[4] | ((uint32_t)tiff[5] << 8) | ((uint32_t)tiff[6] << 16) | ((uint32_t)tiff[7] << 24)
						: ((uint32_t)tiff[4] << 24) | ((uint32_t)tiff[5] << 16) | ((uint32_t)tiff[6] << 8) | (uint32_t)tiff[7];
	if (ifd0 + 2 > size - base) {
		return 1;
	}

	uint32_t count = little
						 ? (uint32_t)tiff[ifd0] | ((uint32_t)tiff[ifd0 + 1] << 8)
						 : ((uint32_t)tiff[ifd0] << 8) | (uint32_t)tiff[ifd0 + 1];

	for (uint32_t i = 0; i < count; i++) {
		size_t entry = ifd0 + 2 + i * 12;
		if (entry + 12 > size - base) {
			break;
		}
		uint32_t tag = little
						   ? (uint32_t)tiff[entry] | ((uint32_t)tiff[entry + 1] << 8)
						   : ((uint32_t)tiff[entry] << 8) | (uint32_t)tiff[entry + 1];
		if (tag != 0x0112) {
			continue;
		}
		uint32_t v = little
						 ? (uint32_t)tiff[entry + 8] | ((uint32_t)tiff[entry + 9] << 8)
						 : ((uint32_t)tiff[entry + 8] << 8) | (uint32_t)tiff[entry + 9];
		return (v >= 1 && v <= 8) ? v : 1;
	}
	return 1;
}

codecs_image_object *codecs_jxl_decode(const uint8_t *data, size_t size, zval *options_zv)
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

	/* Parse options */
	bool first_frame_only = false;
	bool keep_orientation = false;
	bool ignore_icc = false;
	bool ignore_exif = false;
	bool ignore_xmp = false;
	int64_t max_dim = CODECS_G(max_dimension);
	int64_t max_pix = CODECS_G(max_pixels);
	int threads = (int)CODECS_G(threads);

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
		/* Shared DecodeOptions fields: effective on AVIF; no metadata-box
		 * collection happens in the JXL decode pass in v0.1.0. */
		(void)ignore_exif;
		(void)ignore_xmp;
		prop = zend_read_property(codecs_decode_options_ce, Z_OBJ_P(options_zv), "maxDimension", sizeof("maxDimension") - 1, 1, &rv);
		if (prop && Z_TYPE_P(prop) == IS_LONG && Z_LVAL_P(prop) > 0 && Z_LVAL_P(prop) < max_dim) {
			max_dim = Z_LVAL_P(prop);
		}
		prop = zend_read_property(codecs_decode_options_ce, Z_OBJ_P(options_zv), "maxPixels", sizeof("maxPixels") - 1, 1, &rv);
		if (prop && Z_TYPE_P(prop) == IS_LONG && Z_LVAL_P(prop) > 0 && Z_LVAL_P(prop) < max_pix) {
			max_pix = Z_LVAL_P(prop);
		}
		prop = zend_read_property(codecs_decode_options_ce, Z_OBJ_P(options_zv), "threads", sizeof("threads") - 1, 1, &rv);
		if (prop && Z_TYPE_P(prop) == IS_LONG && Z_LVAL_P(prop) > 0) {
			threads = (int)Z_LVAL_P(prop);
		}
	}

	JxlDecoder *dec = JxlDecoderCreate(NULL);
	if (!dec) {
		codecs_throw(codecs_decode_exception_ce, CODECS_ERR_OUT_OF_MEMORY, "Failed to allocate JPEG XL decoder");
		return NULL;
	}

	/* Per-call statelessness: a private thread runner, destroyed with the decoder. */
	void *runner = NULL;
	if (threads > 1) {
		runner = JxlThreadParallelRunnerCreate(NULL, (size_t)threads);
		JxlDecoderSetParallelRunner(dec, JxlThreadParallelRunner, runner);
	}

	/* Subscribe to the event types the decode loop handles. */
	JxlDecoderSubscribeEvents(dec, JXL_DEC_BASIC_INFO | JXL_DEC_COLOR_ENCODING | JXL_DEC_FULL_IMAGE);
	JxlDecoderSetKeepOrientation(dec, keep_orientation ? JXL_TRUE : JXL_FALSE);
	/* Straight-alpha output contract: un-premultiply associated alpha. */
	JxlDecoderSetUnpremultiplyAlpha(dec, JXL_TRUE);

	JxlDecoder *volatile volatile_dec = dec;
	void *volatile volatile_runner = runner;
	uint16_t *volatile volatile_pixels = NULL;
	uint8_t *volatile volatile_icc = NULL;

	JxlBasicInfo info;
	memset(&info, 0, sizeof(info));
	JxlPixelFormat fmt = {4, JXL_TYPE_UINT16, JXL_NATIVE_ENDIAN, 0};
	uint16_t *pixels = NULL;
	size_t buffer_size = 0;
	codecs_cicp cicp;
	memset(&cicp, 0, sizeof(cicp));
	uint8_t *icc = NULL;
	size_t icc_size = 0;
	bool have_color = false;
	uint32_t w = 0, h = 0, src_depth = 8;
	bool have_alpha = false;
	bool got_frame = false;
	uint32_t intrinsic_orientation = 1;

	char errbuf[192];
	errbuf[0] = '\0';

	JxlDecoderSetInput(dec, data, size);
	JxlDecoderCloseInput(dec);

	for (;;) {
		JxlDecoderStatus st;
		zend_try
		{
			st = JxlDecoderProcessInput(dec);
		}
		zend_catch
		{
			if (volatile_pixels) {
				efree((void *)volatile_pixels);
			}
			if (volatile_icc) {
				efree((void *)volatile_icc);
			}
			if (volatile_dec) {
				JxlDecoderDestroy((JxlDecoder *)volatile_dec);
			}
			if (volatile_runner) {
				JxlThreadParallelRunnerDestroy((void *)volatile_runner);
			}
			zend_bailout();
		}
		zend_end_try();

		if (st == JXL_DEC_ERROR) {
			snprintf(errbuf, sizeof(errbuf), "JPEG XL decoder error");
			break;
		}
		if (st == JXL_DEC_NEED_MORE_INPUT) {
			snprintf(errbuf, sizeof(errbuf), "Truncated JPEG XL codestream");
			break;
		}

		if (st == JXL_DEC_BASIC_INFO) {
			if (JxlDecoderGetBasicInfo(dec, &info) != JXL_DEC_SUCCESS) {
				snprintf(errbuf, sizeof(errbuf), "Failed to read JPEG XL basic info");
				break;
			}

			/* Float samples are rejected in v0.1.0 (design record §0). */
			if (info.exponent_bits_per_sample > 0 || info.alpha_exponent_bits > 0) {
				snprintf(errbuf, sizeof(errbuf), "Float-sample JPEG XL images are not supported");
				break;
			}

			w = info.xsize;
			h = info.ysize;
			src_depth = info.bits_per_sample;
			have_alpha = info.alpha_bits > 0;
			if (info.orientation >= 1 && info.orientation <= 8) {
				intrinsic_orientation = (uint32_t)info.orientation;
			}

			if (w == 0 || h == 0 || (int64_t)w > max_dim || (int64_t)h > max_dim) {
				snprintf(errbuf, sizeof(errbuf), "Image dimensions (%ux%u) exceed max dimension limit (%ld)", w, h, max_dim);
				break;
			}
			if ((uint64_t)w * (uint64_t)h > (uint64_t)max_pix) {
				snprintf(errbuf, sizeof(errbuf), "Image pixel count (%lu) exceeds max pixels limit (%ld)", (uint64_t)w * (uint64_t)h, max_pix);
				break;
			}

			/* Animation fail-closed (design record §0). */
			if (info.have_animation && !first_frame_only) {
				snprintf(errbuf, sizeof(errbuf), "Animated JPEG XL is not supported unless firstFrameOnly option is true");
				break;
			}

			/* RGBA 4 channels always (16-bit RGBA storage contract). */
			fmt.num_channels = 4;
			buffer_size = (size_t)w * (size_t)h * 8;
			pixels = emalloc(buffer_size);
			volatile_pixels = pixels;
			continue;
		}

		if (st == JXL_DEC_COLOR_ENCODING) {
			JxlColorEncoding ce;
			have_color = (JxlDecoderGetColorAsEncodedProfile(dec, JXL_COLOR_PROFILE_TARGET_DATA, &ce) == JXL_DEC_SUCCESS);
			if (have_color) {
				codecs_jxl_cicp(&ce, &cicp);
			} else {
				cicp.colour_primaries = 2;
				cicp.transfer_characteristics = 2;
				cicp.matrix_coefficients = 0;
				cicp.full_range = true;
			}
			if (!ignore_icc) {
				codecs_jxl_get_icc(dec, &icc, &icc_size);
				volatile_icc = icc;
			}
			continue;
		}

		if (st == JXL_DEC_NEED_IMAGE_OUT_BUFFER) {
			size_t needed = 0;
			if (JxlDecoderImageOutBufferSize(dec, &fmt, &needed) != JXL_DEC_SUCCESS || needed != buffer_size) {
				snprintf(errbuf, sizeof(errbuf), "JPEG XL output buffer size mismatch (got %zu, expected %zu)", needed, buffer_size);
				break;
			}
			if (JxlDecoderSetImageOutBuffer(dec, &fmt, pixels, buffer_size) != JXL_DEC_SUCCESS) {
				snprintf(errbuf, sizeof(errbuf), "Failed to set JPEG XL output buffer");
				break;
			}
			continue;
		}

		if (st == JXL_DEC_FULL_IMAGE) {
			got_frame = true;
			/* v0.1.0 decodes the first frame only; stop before further frames. */
			break;
		}

		if (st == JXL_DEC_SUCCESS) {
			break;
		}

		/* Metadata boxes and any other unhandled events are skipped by
		 * continuing the loop. */
	}

	if (errbuf[0] != '\0' || !got_frame) {
		if (pixels) {
			efree(pixels);
		}
		if (icc) {
			efree(icc);
		}
		JxlDecoderDestroy(dec);
		if (runner) {
			JxlThreadParallelRunnerDestroy(runner);
		}

		if (!got_frame && errbuf[0] == '\0') {
			codecs_throw(codecs_corrupt_data_exception_ce, CODECS_ERR_BITSTREAM, "JPEG XL codestream contained no frame");
			return NULL;
		}

		/* Map the error text to the typed exception. */
		if (strstr(errbuf, "dimensions") != NULL) {
			codecs_throw(codecs_limit_exception_ce, CODECS_ERR_DIMENSION, "%s", errbuf);
		} else if (strstr(errbuf, "pixel count") != NULL) {
			codecs_throw(codecs_limit_exception_ce, CODECS_ERR_PIXELS, "%s", errbuf);
		} else if (strstr(errbuf, "Animated") != NULL) {
			codecs_throw(codecs_unsupported_format_exception_ce, CODECS_ERR_UNSUPPORTED, "%s", errbuf);
		} else if (strstr(errbuf, "Float-sample") != NULL) {
			codecs_throw(codecs_unsupported_format_exception_ce, CODECS_ERR_UNSUPPORTED, "%s", errbuf);
		} else if (strstr(errbuf, "Truncated") != NULL) {
			codecs_throw(codecs_corrupt_data_exception_ce, CODECS_ERR_BITSTREAM, "%s", errbuf);
		} else {
			codecs_throw(codecs_corrupt_data_exception_ce, CODECS_ERR_BITSTREAM, "%s", errbuf);
		}
		return NULL;
	}

	/* Synthesize opaque alpha when the source had none (3-channel decode). */
	if (!have_alpha) {
		for (uint32_t y = 0; y < h; y++) {
			for (uint32_t x = 0; x < w; x++) {
				pixels[((size_t)y * w + x) * 4 + 3] = 0xFFFF;
			}
		}
	}

	zval img_zv;
	object_init_ex(&img_zv, codecs_image_ce);
	codecs_image_object *img = Z_CODECS_IMAGE_P(&img_zv);

	img->pixels = pixels;
	img->width = w;
	img->height = h;
	img->bit_depth = src_depth;
	img->alpha_mode = 0; /* straight */
	img->provenance = estrdup("jxl");
	img->cicp.colour_primaries = cicp.colour_primaries;
	img->cicp.transfer_characteristics = cicp.transfer_characteristics;
	img->cicp.matrix_coefficients = cicp.matrix_coefficients;
	img->cicp.full_range = cicp.full_range;

	if (icc && icc_size > 0 && icc_size <= (size_t)CODECS_G(max_metadata_bytes)) {
		img->icc_profile = icc;
		img->icc_size = icc_size;
	} else if (icc) {
		efree(icc);
	}

	/* Orientation: with keep_orientation the pixel buffer keeps the intrinsic
	 * codestream orientation; otherwise it has been applied, so report TopLeft. */
	img->orientation = keep_orientation ? intrinsic_orientation : 1;

	JxlDecoderDestroy(dec);
	if (runner) {
		JxlThreadParallelRunnerDestroy(runner);
	}
	return img;
}

bool codecs_jxl_probe(const uint8_t *data, size_t size, zval *return_value)
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

	JxlDecoder *dec = JxlDecoderCreate(NULL);
	if (!dec) {
		codecs_throw(codecs_decode_exception_ce, CODECS_ERR_OUT_OF_MEMORY, "Failed to allocate JPEG XL decoder");
		return false;
	}

	/* Raw container dimensions + original orientation (design record §0). */
	JxlDecoderSubscribeEvents(dec, JXL_DEC_BASIC_INFO | JXL_DEC_COLOR_ENCODING | JXL_DEC_BOX);
	JxlDecoderSetKeepOrientation(dec, JXL_TRUE);
	JxlDecoderSetDecompressBoxes(dec, JXL_TRUE);
	JxlDecoderSetInput(dec, data, size);
	JxlDecoderCloseInput(dec);

	JxlBasicInfo info;
	memset(&info, 0, sizeof(info));
	codecs_cicp cicp;
	memset(&cicp, 0, sizeof(cicp));
	uint8_t *icc = NULL;
	size_t icc_size = 0;
	uint8_t *exif = NULL;
	size_t exif_size = 0;
	uint8_t *xmp = NULL;
	size_t xmp_size = 0;

	/* Accumulation state for the metadata box currently being collected. */
	uint8_t *box_buf = NULL;
	size_t box_cap = 0;
	size_t box_have = 0;
	size_t chunk_off = 0;
	size_t chunk_cap = 0;
	uint8_t **dest = NULL;
	size_t *dest_size = NULL;

	char errbuf[192];
	errbuf[0] = '\0';

	for (;;) {
		JxlDecoderStatus st = JxlDecoderProcessInput(dec);

		if (st == JXL_DEC_ERROR) {
			snprintf(errbuf, sizeof(errbuf), "JPEG XL decoder error");
			break;
		}
		if (st == JXL_DEC_NEED_MORE_INPUT) {
			snprintf(errbuf, sizeof(errbuf), "Truncated JPEG XL codestream");
			break;
		}

		if (st == JXL_DEC_BASIC_INFO) {
			if (JxlDecoderGetBasicInfo(dec, &info) != JXL_DEC_SUCCESS) {
				snprintf(errbuf, sizeof(errbuf), "Failed to read JPEG XL basic info");
				break;
			}
			continue;
		}

		if (st == JXL_DEC_COLOR_ENCODING) {
			JxlColorEncoding ce;
			if (JxlDecoderGetColorAsEncodedProfile(dec, JXL_COLOR_PROFILE_TARGET_DATA, &ce) == JXL_DEC_SUCCESS) {
				codecs_jxl_cicp(&ce, &cicp);
			} else {
				cicp.colour_primaries = 2;
				cicp.transfer_characteristics = 2;
				cicp.matrix_coefficients = 0;
				cicp.full_range = true;
			}
			codecs_jxl_get_icc(dec, &icc, &icc_size);
			continue;
		}

		if (st == JXL_DEC_BOX || st == JXL_DEC_BOX_NEED_MORE_OUTPUT) {
			/* Close out the previous chunk: count the bytes actually written. */
			size_t unfilled = JxlDecoderReleaseBoxBuffer(dec);
			box_have += chunk_cap - unfilled;
			if (dest_size) {
				*dest_size = box_have;
			}

			if (st == JXL_DEC_BOX) {
				box_buf = NULL;
				box_cap = box_have = chunk_off = chunk_cap = 0;
				dest = NULL;
				dest_size = NULL;

				JxlBoxType type;
				if (JxlDecoderGetBoxType(dec, type, JXL_TRUE) != JXL_DEC_SUCCESS) {
					continue;
				}
				size_t box_size = 0;
				if (JxlDecoderGetBoxSizeContents(dec, &box_size) != JXL_DEC_SUCCESS ||
					box_size == 0 || box_size > (size_t)CODECS_G(max_metadata_bytes)) {
					continue;
				}
				if (memcmp(type, "Exif", 4) == 0) {
					dest = &exif;
					dest_size = &exif_size;
				} else if (memcmp(type, "xml ", 4) == 0) {
					dest = &xmp;
					dest_size = &xmp_size;
				} else {
					continue;
				}
				box_cap = box_size;
				box_buf = emalloc(box_cap);
				if (*dest) {
					efree(*dest);
				}
				*dest = box_buf;
				*dest_size = 0;
			} else if (box_buf) {
				/* NEED_MORE_OUTPUT: the decompressed payload outgrew the
				 * buffer. Grow, bounded by the metadata ceiling. */
				if (box_cap >= (size_t)CODECS_G(max_metadata_bytes)) {
					efree(box_buf);
					if (dest) {
						*dest = NULL;
					}
					if (dest_size) {
						*dest_size = 0;
					}
					box_buf = NULL;
					box_cap = box_have = chunk_off = chunk_cap = 0;
					dest = NULL;
					dest_size = NULL;
					continue; /* no buffer set = decoder skips the rest of the box */
				}
				box_cap = (box_cap * 2 < (size_t)CODECS_G(max_metadata_bytes)) ? box_cap * 2 : (size_t)CODECS_G(max_metadata_bytes);
				box_buf = erealloc(box_buf, box_cap);
				if (dest) {
					*dest = box_buf;
				}
			} else {
				continue;
			}

			chunk_off = box_have;
			chunk_cap = box_cap - box_have;
			if (JxlDecoderSetBoxBuffer(dec, box_buf + chunk_off, chunk_cap) != JXL_DEC_SUCCESS) {
				efree(box_buf);
				if (dest) {
					*dest = NULL;
				}
				if (dest_size) {
					*dest_size = 0;
				}
				box_buf = NULL;
				box_cap = box_have = chunk_off = chunk_cap = 0;
				dest = NULL;
				dest_size = NULL;
				continue;
			}
			continue;
		}

		if (st == JXL_DEC_SUCCESS) {
			break;
		}
	}

	/* Finalize the last metadata box: count the bytes actually written. */
	size_t unfilled = JxlDecoderReleaseBoxBuffer(dec);
	box_have += chunk_cap - unfilled;
	if (dest_size) {
		*dest_size = box_have;
	}

	if (errbuf[0] != '\0') {
		if (icc) {
			efree(icc);
		}
		if (exif) {
			efree(exif);
		}
		if (xmp) {
			efree(xmp);
		}
		JxlDecoderDestroy(dec);
		if (strstr(errbuf, "Truncated") != NULL) {
			codecs_throw(codecs_corrupt_data_exception_ce, CODECS_ERR_BITSTREAM, "%s", errbuf);
		} else {
			codecs_throw(codecs_corrupt_data_exception_ce, CODECS_ERR_BITSTREAM, "%s", errbuf);
		}
		return false;
	}

	/* Build Codecs\Info */
	object_init_ex(return_value, codecs_info_ce);

	zend_object *format_obj = zend_enum_get_case_cstr(codecs_format_ce, "JpegXl");
	if (format_obj) {
		zval zv;
		ZVAL_OBJ(&zv, format_obj);
		zend_update_property(codecs_info_ce, Z_OBJ_P(return_value), "format", sizeof("format") - 1, &zv);
	}

	zend_update_property_long(codecs_info_ce, Z_OBJ_P(return_value), "width", sizeof("width") - 1, info.xsize);
	zend_update_property_long(codecs_info_ce, Z_OBJ_P(return_value), "height", sizeof("height") - 1, info.ysize);
	zend_update_property_long(codecs_info_ce, Z_OBJ_P(return_value), "bitDepth", sizeof("bitDepth") - 1, info.bits_per_sample);
	zend_update_property_bool(codecs_info_ce, Z_OBJ_P(return_value), "alpha", sizeof("alpha") - 1, info.alpha_bits > 0);

	zval cicp_zv;
	object_init_ex(&cicp_zv, codecs_cicp_ce);
	zend_update_property_long(codecs_cicp_ce, Z_OBJ(cicp_zv), "colourPrimaries", sizeof("colourPrimaries") - 1, cicp.colour_primaries);
	zend_update_property_long(codecs_cicp_ce, Z_OBJ(cicp_zv), "transferCharacteristics", sizeof("transferCharacteristics") - 1, cicp.transfer_characteristics);
	zend_update_property_long(codecs_cicp_ce, Z_OBJ(cicp_zv), "matrixCoefficients", sizeof("matrixCoefficients") - 1, cicp.matrix_coefficients);
	zend_update_property_bool(codecs_cicp_ce, Z_OBJ(cicp_zv), "fullRange", sizeof("fullRange") - 1, cicp.full_range);
	zend_update_property(codecs_info_ce, Z_OBJ_P(return_value), "cicp", sizeof("cicp") - 1, &cicp_zv);
	zval_ptr_dtor(&cicp_zv);

	if (icc && icc_size > 0) {
		zend_update_property_stringl(codecs_info_ce, Z_OBJ_P(return_value), "iccProfile", sizeof("iccProfile") - 1, (char *)icc, icc_size);
	} else {
		zend_update_property_null(codecs_info_ce, Z_OBJ_P(return_value), "iccProfile", sizeof("iccProfile") - 1);
	}

	/* Orientation: prefer the intrinsic codestream orientation, falling
	 * back to the Exif box only when the codestream reports identity (1). */
	uint32_t orient = (info.orientation >= 1 && info.orientation <= 8) ? (uint32_t)info.orientation : 1;
	if (orient == 1 && exif && exif_size > 0) {
		orient = codecs_jxl_exif_orientation(exif, exif_size);
	}
	zend_object *orient_obj = NULL;
	if (zend_enum_get_case_by_value(&orient_obj, codecs_orientation_ce, orient, NULL, false) == SUCCESS && orient_obj) {
		zval zv;
		ZVAL_OBJ(&zv, orient_obj);
		zend_update_property(codecs_info_ce, Z_OBJ_P(return_value), "orientation", sizeof("orientation") - 1, &zv);
	}

	uint32_t frames = info.have_animation ? 2 : 1;
	zend_update_property_long(codecs_info_ce, Z_OBJ_P(return_value), "frameCount", sizeof("frameCount") - 1, frames);
	zend_update_property_bool(codecs_info_ce, Z_OBJ_P(return_value), "animated", sizeof("animated") - 1, info.have_animation ? true : false);
	zend_update_property_long(codecs_info_ce, Z_OBJ_P(return_value), "byteSize", sizeof("byteSize") - 1, size);

	zval blocks_zv;
	array_init(&blocks_zv);
	if (exif && exif_size > 0) {
		add_assoc_stringl(&blocks_zv, "exif", (char *)exif, exif_size);
	}
	if (xmp && xmp_size > 0) {
		add_assoc_stringl(&blocks_zv, "xmp", (char *)xmp, xmp_size);
	}
	zend_update_property(codecs_info_ce, Z_OBJ_P(return_value), "metadataBlocks", sizeof("metadataBlocks") - 1, &blocks_zv);
	zval_ptr_dtor(&blocks_zv);

	if (icc) {
		efree(icc);
	}
	if (exif) {
		efree(exif);
	}
	if (xmp) {
		efree(xmp);
	}
	JxlDecoderDestroy(dec);
	return true;
}

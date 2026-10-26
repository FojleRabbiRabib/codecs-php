/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2026 Fojle Rabbi (Rabib)
 *
 * dump-pixels — oracle tool for byte-identical raw-pixel decode comparison.
 * Compiles against the exact same vendored static libraries (libavif/libdav1d
 * and libjxl/highway/brotli/skcms). Reads an AVIF or JXL file and writes raw
 * 16-bit RGBA bytes to stdout (or file).
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "avif/avif.h"
#include "jxl/decode.h"
#include "jxl/types.h"

static int dump_avif(const char *input_path, uint16_t **pixels_out, size_t *size_out)
{
	avifDecoder *decoder = avifDecoderCreate();
	if (!decoder) {
		fprintf(stderr, "FAIL: avifDecoderCreate\n");
		return 2;
	}

	decoder->maxThreads = 1;
	decoder->strictFlags = AVIF_STRICT_ENABLED;

	avifResult result = avifDecoderSetIOFile(decoder, input_path);
	if (result != AVIF_RESULT_OK) {
		fprintf(stderr, "FAIL: set IO file %s: %s\n", input_path, avifResultToString(result));
		avifDecoderDestroy(decoder);
		return 3;
	}

	result = avifDecoderParse(decoder);
	if (result != AVIF_RESULT_OK) {
		fprintf(stderr, "FAIL: parse %s: %s\n", input_path, avifResultToString(result));
		avifDecoderDestroy(decoder);
		return 4;
	}

	result = avifDecoderNextImage(decoder);
	if (result != AVIF_RESULT_OK) {
		fprintf(stderr, "FAIL: nextImage %s: %s\n", input_path, avifResultToString(result));
		avifDecoderDestroy(decoder);
		return 5;
	}

	avifImage *image = decoder->image;
	uint32_t w = image->width;
	uint32_t h = image->height;

	avifRGBImage rgb;
	avifRGBImageSetDefaults(&rgb, image);
	rgb.depth = 16;
	rgb.format = AVIF_RGB_FORMAT_RGBA;
	rgb.alphaPremultiplied = AVIF_FALSE;

	size_t buffer_size = (size_t)w * (size_t)h * 8;
	uint16_t *pixels = malloc(buffer_size);
	if (!pixels) {
		fprintf(stderr, "FAIL: malloc %zu\n", buffer_size);
		avifDecoderDestroy(decoder);
		return 6;
	}

	rgb.pixels = (uint8_t *)pixels;
	rgb.rowBytes = w * 8;

	result = avifImageYUVToRGB(image, &rgb);
	if (result != AVIF_RESULT_OK) {
		fprintf(stderr, "FAIL: YUV to RGB: %s\n", avifResultToString(result));
		free(pixels);
		avifDecoderDestroy(decoder);
		return 7;
	}

	/* Invert/rotate orientation to match extension (B2 + S7) */
	if (image->transformFlags & (AVIF_TRANSFORM_IROT | AVIF_TRANSFORM_IMIR)) {
		if ((image->transformFlags & AVIF_TRANSFORM_IROT) && image->irot.angle == 2) {
			size_t total_pixels = (size_t)w * (size_t)h;
			for (size_t i = 0; i < total_pixels / 2; i++) {
				size_t j = total_pixels - 1 - i;
				for (int c = 0; c < 4; c++) {
					uint16_t tmp = pixels[i * 4 + c];
					pixels[i * 4 + c] = pixels[j * 4 + c];
					pixels[j * 4 + c] = tmp;
				}
			}
		} else if ((image->transformFlags & AVIF_TRANSFORM_IROT) && (image->irot.angle == 1 || image->irot.angle == 3)) {
			uint32_t new_w = h;
			uint32_t new_h = w;
			size_t new_bytes = (size_t)new_w * (size_t)new_h * 8;
			uint16_t *dst = malloc(new_bytes);
			for (uint32_t y = 0; y < h; y++) {
				for (uint32_t x = 0; x < w; x++) {
					uint32_t dst_x, dst_y;
					if (image->irot.angle == 3) {
						dst_x = h - 1 - y;
						dst_y = x;
					} else {
						dst_x = y;
						dst_y = w - 1 - x;
					}
					size_t src_idx = ((size_t)y * w + x) * 4;
					size_t dst_idx = ((size_t)dst_y * new_w + dst_x) * 4;
					for (int c = 0; c < 4; c++) {
						dst[dst_idx + c] = pixels[src_idx + c];
					}
				}
			}
			free(pixels);
			pixels = dst;
			w = new_w;
			h = new_h;
			buffer_size = new_bytes;
		}

		if (image->transformFlags & AVIF_TRANSFORM_IMIR) {
			if (image->imir.axis == 1) {
				for (uint32_t y = 0; y < h; y++) {
					for (uint32_t x = 0; x < w / 2; x++) {
						size_t i = ((size_t)y * w + x) * 4;
						size_t j = ((size_t)y * w + (w - 1 - x)) * 4;
						for (int c = 0; c < 4; c++) {
							uint16_t tmp = pixels[i + c];
							pixels[i + c] = pixels[j + c];
							pixels[j + c] = tmp;
						}
					}
				}
			} else {
				for (uint32_t y = 0; y < h / 2; y++) {
					for (uint32_t x = 0; x < w; x++) {
						size_t i = ((size_t)y * w + x) * 4;
						size_t j = ((size_t)(h - 1 - y) * w + x) * 4;
						for (int c = 0; c < 4; c++) {
							uint16_t tmp = pixels[i + c];
							pixels[i + c] = pixels[j + c];
							pixels[j + c] = tmp;
						}
					}
				}
			}
		}
	}

	avifDecoderDestroy(decoder);
	*pixels_out = pixels;
	*size_out = buffer_size;
	return 0;
}

static int dump_jxl(const char *input_path, uint16_t **pixels_out, size_t *size_out)
{
	FILE *f = fopen(input_path, "rb");
	if (!f) {
		fprintf(stderr, "FAIL: open input %s\n", input_path);
		return 3;
	}
	fseek(f, 0, SEEK_END);
	size_t file_size = ftell(f);
	fseek(f, 0, SEEK_SET);

	uint8_t *data = malloc(file_size);
	if (!data) {
		fclose(f);
		return 4;
	}
	if (fread(data, 1, file_size, f) != file_size) {
		fclose(f);
		free(data);
		return 5;
	}
	fclose(f);

	JxlDecoder *dec = JxlDecoderCreate(NULL);
	if (!dec) {
		free(data);
		return 6;
	}

	JxlDecoderSubscribeEvents(dec, JXL_DEC_BASIC_INFO | JXL_DEC_COLOR_ENCODING | JXL_DEC_FULL_IMAGE);
	/* Match the extension's decoder configuration exactly (straight alpha). */
	JxlDecoderSetUnpremultiplyAlpha(dec, JXL_TRUE);
	JxlDecoderSetInput(dec, data, file_size);
	JxlDecoderCloseInput(dec);

	JxlPixelFormat fmt = {4, JXL_TYPE_UINT16, JXL_NATIVE_ENDIAN, 0};
	uint16_t *pixels = NULL;
	size_t buffer_size = 0;

	for (;;) {
		JxlDecoderStatus st = JxlDecoderProcessInput(dec);
		if (st == JXL_DEC_ERROR || st == JXL_DEC_NEED_MORE_INPUT) {
			fprintf(stderr, "FAIL: JXL decoder error %d\n", st);
			free(data);
			if (pixels)
				free(pixels);
			JxlDecoderDestroy(dec);
			return 7;
		}
		if (st == JXL_DEC_BASIC_INFO) {
			JxlBasicInfo info;
			if (JxlDecoderGetBasicInfo(dec, &info) != JXL_DEC_SUCCESS) {
				free(data);
				JxlDecoderDestroy(dec);
				return 8;
			}
			buffer_size = (size_t)info.xsize * (size_t)info.ysize * 8;
			pixels = malloc(buffer_size);
			continue;
		}
		if (st == JXL_DEC_NEED_IMAGE_OUT_BUFFER) {
			if (JxlDecoderSetImageOutBuffer(dec, &fmt, pixels, buffer_size) != JXL_DEC_SUCCESS) {
				free(data);
				free(pixels);
				JxlDecoderDestroy(dec);
				return 9;
			}
			continue;
		}
		if (st == JXL_DEC_FULL_IMAGE || st == JXL_DEC_SUCCESS) {
			break;
		}
	}

	JxlDecoderDestroy(dec);
	free(data);
	*pixels_out = pixels;
	*size_out = buffer_size;
	return 0;
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <input.(avif|jxl)> [output.raw]\n", argv[0]);
		return 1;
	}

	const char *input_path = argv[1];
	const char *output_path = (argc >= 3) ? argv[2] : NULL;

	uint16_t *pixels = NULL;
	size_t buffer_size = 0;
	int ret = 0;

	size_t len = strlen(input_path);
	if (len > 4 && strcmp(input_path + len - 4, ".jxl") == 0) {
		ret = dump_jxl(input_path, &pixels, &buffer_size);
	} else {
		ret = dump_avif(input_path, &pixels, &buffer_size);
	}

	if (ret != 0 || !pixels) {
		return ret;
	}

	FILE *out = stdout;
	if (output_path) {
		out = fopen(output_path, "wb");
		if (!out) {
			fprintf(stderr, "FAIL: open output %s\n", output_path);
			free(pixels);
			return 8;
		}
	}

	size_t written = fwrite(pixels, 1, buffer_size, out);
	if (written != buffer_size) {
		fprintf(stderr, "FAIL: short write (%zu of %zu)\n", written, buffer_size);
		if (output_path)
			fclose(out);
		free(pixels);
		return 9;
	}

	if (output_path)
		fclose(out);
	free(pixels);
	return 0;
}

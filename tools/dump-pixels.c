/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2026 Fojle Rabbi (Rabib)
 *
 * dump-pixels — oracle tool for byte-identical raw-pixel decode comparison.
 * Compiles against the exact same vendored libavif/libdav1d static libraries.
 * Reads an AVIF file and writes raw 16-bit RGBA bytes to stdout (or file).
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "avif/avif.h"

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <input.avif> [output.raw]\n", argv[0]);
		return 1;
	}

	const char *input_path = argv[1];
	const char *output_path = (argc >= 3) ? argv[2] : NULL;

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

	FILE *out = stdout;
	if (output_path) {
		out = fopen(output_path, "wb");
		if (!out) {
			fprintf(stderr, "FAIL: open output %s\n", output_path);
			free(pixels);
			avifDecoderDestroy(decoder);
			return 8;
		}
	}

	size_t written = fwrite(pixels, 1, buffer_size, out);
	if (written != buffer_size) {
		fprintf(stderr, "FAIL: short write (%zu of %zu)\n", written, buffer_size);
		if (output_path)
			fclose(out);
		free(pixels);
		avifDecoderDestroy(decoder);
		return 9;
	}

	if (output_path)
		fclose(out);
	free(pixels);
	avifDecoderDestroy(decoder);
	return 0;
}

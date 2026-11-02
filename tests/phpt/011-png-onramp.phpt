--TEST--
Codecs\Image::fromPng and toPng provide lossless PNG round-trip and supported-subset enforcement
--EXTENSIONS--
codecs
--FILE--
<?php
/*
 * Copyright 2026 Fojle Rabbi (Rabib)
 * SPDX-License-Identifier: Apache-2.0
 */

declare(strict_types=1);

// 1. 16-bit RGBA lossless round-trip
$raw16 = str_repeat("\x80\x00\x40\x00\x20\x00\xFF\xFF", 4);
$img16 = Codecs\Image::fromPixels(2, 2, $raw16, Codecs\AlphaMode::Straight);
$png16 = $img16->toPng();
$re16 = Codecs\Image::fromPng($png16);

var_dump(strncmp($png16, "\x89PNG\r\n\x1a\n", 8) === 0);
var_dump($re16->width() === 2);
var_dump($re16->height() === 2);
var_dump($re16->bitDepth() === 16);
var_dump($re16->provenance() === "png");
var_dump($re16->rgba16() === $raw16);

// 2. 8-bit RGBA round-trip via JXL fixture
$jxl = Codecs\Codec::decodeFile(getcwd() . '/tests/fixtures/gradient_8x8_rgba_lossless.jxl');
$png8 = $jxl->toPng();
$re8 = Codecs\Image::fromPng($png8);

var_dump($re8->width() === 8);
var_dump($re8->height() === 8);
var_dump($re8->bitDepth() === 8);
var_dump($re8->rgba8() === $jxl->rgba8());

// 3. 10-bit AVIF exported to PNG preserves 16-bit normalized depth
$avif = Codecs\Codec::decodeFile(getcwd() . '/vendor-src/libavif-1.4.2/tests/data/colors_text_hdr_srgb.avif');
var_dump($avif->bitDepth() === 10);
$pngFrom10b = $avif->toPng();
$reFrom10b = Codecs\Image::fromPng($pngFrom10b);
var_dump($reFrom10b->bitDepth() === 16);

// 4. Transparent dispatch via Codec::decode() and DecodeOptions enforcement
$dispatched = Codecs\Codec::decode($png16);
var_dump($dispatched->width() === 2);
var_dump($dispatched->provenance() === "png");

try {
    Codecs\Codec::decode($png16, new Codecs\DecodeOptions(maxDimension: 1));
} catch (Codecs\LimitException $e) {
    echo "maxDimension on PNG: " . $e->getErrorCode()->name . "\n";
}

// 5. Unsupported subset rejection: grayscale PNG
function make_png(int $color_type, int $interlace): string {
    $ihdr = pack("NNCCCCC", 1, 1, 8, $color_type, 0, 0, $interlace);
    $raw_bytes = ($color_type === 0) ? "\x00\x80" : "\x00\x80\x40\x20";
    $idat = gzcompress($raw_bytes);
    return "\x89PNG\r\n\x1a\n"
        . pack("N", strlen($ihdr)) . "IHDR" . $ihdr . pack("N", crc32("IHDR" . $ihdr))
        . pack("N", strlen($idat)) . "IDAT" . $idat . pack("N", crc32("IDAT" . $idat))
        . pack("N", 0) . "IEND" . pack("N", crc32("IEND"));
}

try {
    Codecs\Image::fromPng(make_png(0, 0)); // color_type 0 = grayscale
} catch (Codecs\UnsupportedFormatException $e) {
    echo "grayscale rejected: " . $e->getErrorCode()->name . "\n";
}

try {
    Codecs\Image::fromPng(make_png(2, 1)); // interlace 1 = Adam7
} catch (Codecs\UnsupportedFormatException $e) {
    echo "interlaced rejected: " . $e->getErrorCode()->name . "\n";
}

// 6. Invalid / corrupt inputs
try {
    Codecs\Image::fromPng("");
} catch (Codecs\CorruptDataException $e) {
    echo "empty png: " . $e->getErrorCode()->name . "\n";
}

try {
    Codecs\Image::fromPng("not-a-valid-png-signature");
} catch (Codecs\UnsupportedFormatException $e) {
    echo "invalid signature: " . $e->getErrorCode()->name . "\n";
}
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
maxDimension on PNG: Dimension
grayscale rejected: Unsupported
interlaced rejected: Unsupported
empty png: Bitstream
invalid signature: Unsupported

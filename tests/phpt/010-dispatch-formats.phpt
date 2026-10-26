--TEST--
Codecs\Codec::dispatch decodes both AVIF and JPEG XL by container sniffing
--EXTENSIONS--
codecs
--FILE--
<?php
/*
 * Copyright 2026 Fojle Rabbi (Rabib)
 * SPDX-License-Identifier: Apache-2.0
 */

declare(strict_types=1);

$avif = getcwd() . '/vendor-src/libavif-1.4.2/tests/data/white_1x1.avif';
$jxl = getcwd() . '/tests/fixtures/white_1x1_rgba_lossless.jxl';

$avifImg = Codecs\Codec::decodeFile($avif);
$jxlImg = Codecs\Codec::decodeFile($jxl);

var_dump($avifImg->provenance() === "avif");
var_dump($jxlImg->provenance() === "jxl");
var_dump($avifImg->width() === 1);
var_dump($jxlImg->width() === 1);

// Garbage signature throws UnsupportedFormatException
try {
    Codecs\Codec::decode("not-an-image-at-all-000000");
    echo "FAIL: expected UnsupportedFormatException\n";
} catch (Codecs\UnsupportedFormatException $e) {
    echo "unknown format: " . $e->getErrorCode()->name . "\n";
}
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
unknown format: Unsupported

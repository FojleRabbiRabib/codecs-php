--TEST--
Codecs\Codec::probe inspects JPEG XL container and codestream metadata
--EXTENSIONS--
codecs
--FILE--
<?php
/*
 * Copyright 2026 Fojle Rabbi (Rabib)
 * SPDX-License-Identifier: Apache-2.0
 */

declare(strict_types=1);

$file = getcwd() . '/tests/fixtures/gradient_8x8_rgba_lossless.jxl';
$info = Codecs\Codec::probeFile($file);

var_dump($info instanceof Codecs\Info);
var_dump($info->format === Codecs\Format::JpegXl);
var_dump($info->width === 8);
var_dump($info->height === 8);
var_dump($info->bitDepth === 8);
var_dump($info->alpha === true);
var_dump($info->frameCount === 1);
var_dump($info->animated === false);
var_dump($info->byteSize > 0);
var_dump($info->orientation === Codecs\Orientation::TopLeft);

// Probe lossy RGB file
$lossy = getcwd() . '/tests/fixtures/gradient_64x64_rgb_lossy.jxl';
$infoLossy = Codecs\Codec::probeFile($lossy);
var_dump($infoLossy->width === 64);
var_dump($infoLossy->height === 64);
var_dump($infoLossy->alpha === false);

// Probe a brob-compressed XMP box (NEED_MORE_OUTPUT growth path)
$brob = getcwd() . '/tests/fixtures/xmp_brob_8x8_lossless.jxl';
$infoBrob = Codecs\Codec::probeFile($brob);
var_dump(isset($infoBrob->metadataBlocks['xmp']));
var_dump(strlen($infoBrob->metadataBlocks['xmp']) === 318);
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
bool(true)

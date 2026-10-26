--TEST--
Codecs\Codec::decode decodes JPEG XL bitstreams to 16-bit RGBA Image
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
$img = Codecs\Codec::decodeFile($file);

var_dump($img->width() === 8);
var_dump($img->height() === 8);
var_dump($img->bitDepth() === 8);
var_dump($img->alphaMode() === Codecs\AlphaMode::Straight);
var_dump($img->provenance() === "jxl");
var_dump(strlen($img->rgba16()) === 512);
var_dump(strlen($img->rgba8()) === 256);

$cicp = $img->cicp();
var_dump($cicp instanceof Codecs\Cicp);
var_dump($cicp->fullRange === true);

// Lossy image without alpha synthesizes straight opaque alpha
$lossy = getcwd() . '/tests/fixtures/gradient_64x64_rgb_lossy.jxl';
$imgLossy = Codecs\Codec::decodeFile($lossy);
var_dump($imgLossy->width() === 64);
var_dump($imgLossy->height() === 64);
var_dump(strlen($imgLossy->rgba16()) === 32768);
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

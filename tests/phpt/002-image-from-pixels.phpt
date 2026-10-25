--TEST--
Codecs\Image::fromPixels creates an Image from raw RGBA16 buffer with validation
--EXTENSIONS--
codecs
--FILE--
<?php
/*
 * Copyright 2026 Fojle Rabbi (Rabib)
 * SPDX-License-Identifier: Apache-2.0
 */

declare(strict_types=1);

// 2x2 RGBA16 buffer = 2 * 2 * 4 * 2 = 32 bytes
$raw = str_repeat("\x80\x00\x40\x00\x20\x00\xFF\xFF", 4);
$img = Codecs\Image::fromPixels(2, 2, $raw, Codecs\AlphaMode::Straight);

var_dump($img->width());
var_dump($img->height());
var_dump($img->bitDepth());
var_dump($img->alphaMode() === Codecs\AlphaMode::Straight);
var_dump($img->provenance());
var_dump(strlen($img->rgba16()));
var_dump(strlen($img->rgba8()));
var_dump($img->memoryUsage());

// Length mismatch validation
try {
    Codecs\Image::fromPixels(2, 2, "short", Codecs\AlphaMode::Straight);
    echo "FAIL: expected length mismatch\n";
} catch (ValueError $e) {
    echo "length mismatch: " . (strlen($e->getMessage()) > 0 ? "OK" : "empty") . "\n";
}

// Zero dimension validation
try {
    Codecs\Image::fromPixels(0, 2, "", Codecs\AlphaMode::Straight);
    echo "FAIL: expected zero dim\n";
} catch (ValueError $e) {
    echo "zero dim: " . (strlen($e->getMessage()) > 0 ? "OK" : "empty") . "\n";
}

// Close and guards
$img->close();
var_dump($img->memoryUsage() === 0);

try {
    $img->width();
} catch (Error $e) {
    echo "use-after-close: " . $e->getMessage() . "\n";
}

// Clone refusal
try {
    $c = clone $img;
} catch (Error $e) {
    echo "clone-refused: " . (str_contains($e->getMessage(), "uncloneable") ? "OK" : "FAIL") . "\n";
}
?>
--EXPECT--
int(2)
int(2)
int(16)
bool(true)
string(6) "pixels"
int(32)
int(16)
int(32)
length mismatch: OK
zero dim: OK
bool(true)
use-after-close: Cannot operate on a closed Codecs\Image
clone-refused: OK

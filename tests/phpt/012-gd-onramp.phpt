--TEST--
Codecs\Image::fromGd imports truecolor and palette GD images with alpha normalization
--EXTENSIONS--
codecs
gd
--FILE--
<?php
/*
 * Copyright 2026 Fojle Rabbi (Rabib)
 * SPDX-License-Identifier: Apache-2.0
 */

declare(strict_types=1);

// 1. Truecolor RGBA with alpha gradient
$im = imagecreatetruecolor(2, 2);
imagealphablending($im, false);
imagesavealpha($im, true);

$c00 = imagecolorallocatealpha($im, 255, 0, 0, 0);       // opaque red
$c10 = imagecolorallocatealpha($im, 0, 255, 0, 64);      // ~50% green
$c01 = imagecolorallocatealpha($im, 0, 0, 255, 127);     // transparent blue
$c11 = imagecolorallocatealpha($im, 255, 255, 255, 0);   // opaque white

imagesetpixel($im, 0, 0, $c00);
imagesetpixel($im, 1, 0, $c10);
imagesetpixel($im, 0, 1, $c01);
imagesetpixel($im, 1, 1, $c11);

$img = Codecs\Image::fromGd($im);

var_dump($img->width() === 2);
var_dump($img->height() === 2);
var_dump($img->bitDepth() === 8);
var_dump($img->provenance() === "gd");
var_dump($img->alphaMode() === Codecs\AlphaMode::Straight);
var_dump($img->orientation() === Codecs\Orientation::TopLeft);

$rgba16 = $img->rgba16();
var_dump(bin2hex(substr($rgba16, 0, 8)) === "ffff00000000ffff"); // (0,0) red opaque
var_dump(bin2hex(substr($rgba16, 16, 8)) === "00000000ffff0000"); // (0,1) blue transparent
var_dump(bin2hex(substr($rgba16, 24, 8)) === "ffffffffffffffff"); // (1,1) white opaque

// 2. Palette-based image with transparent color index
$pal = imagecreate(2, 2);
$bg = imagecolorallocate($pal, 10, 20, 30);
$fg = imagecolorallocate($pal, 200, 150, 100);
imagecolortransparent($pal, $bg); // $bg is transparent

imagesetpixel($pal, 1, 0, $fg);
$imgPal = Codecs\Image::fromGd($pal);

var_dump($imgPal->provenance() === "gd");
var_dump($imgPal->bitDepth() === 8);
$palRgba = $imgPal->rgba16();
var_dump(bin2hex(substr($palRgba, 0, 8)) === "0a0a14141e1e0000"); // transparent bg
var_dump(bin2hex(substr($palRgba, 8, 8)) === "c8c896966464ffff"); // opaque fg

// Assert caller's GD object is not mutated
var_dump(!imageistruecolor($pal));

// 3. Round-trip through toPng()
$png = $img->toPng();
$reDecoded = Codecs\Image::fromPng($png);
var_dump($reDecoded->width() === 2);
var_dump($reDecoded->rgba8() === $img->rgba8());

// 4. Type error when non-GdImage passed
try {
    Codecs\Image::fromGd(new stdClass());
} catch (TypeError $e) {
    echo "invalid object: " . $e->getMessage() . "\n";
}

// 5. Ceiling enforcement on dimensions (> 16384)
$wide = imagecreatetruecolor(16385, 1);
try {
    Codecs\Image::fromGd($wide);
} catch (Codecs\LimitException $e) {
    echo "dimension limit: " . $e->getErrorCode()->name . "\n";
}

// 6. Ceiling enforcement on pixels (> 16777216)
$huge = imagecreatetruecolor(16384, 1025);
try {
    Codecs\Image::fromGd($huge);
} catch (Codecs\LimitException $e) {
    echo "pixel limit: " . $e->getErrorCode()->name . "\n";
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
bool(true)
bool(true)
invalid object: Argument #1 ($gd) must be an instance of GdImage, stdClass given
dimension limit: Dimension
pixel limit: Pixels

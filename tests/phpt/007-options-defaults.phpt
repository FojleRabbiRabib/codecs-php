--TEST--
Options classes instantiate with default properties and valid enum cases
--EXTENSIONS--
codecs
--FILE--
<?php
/*
 * Copyright 2026 Fojle Rabbi (Rabib)
 * SPDX-License-Identifier: Apache-2.0
 */

declare(strict_types=1);

$avifOpts = new Codecs\AvifEncodeOptions();
var_dump($avifOpts->quality === 60);
var_dump($avifOpts->qualityAlpha === null);
var_dump($avifOpts->speed === 6);
var_dump($avifOpts->subsampling === Codecs\ChromaSubsampling::Yuv420);
var_dump($avifOpts->matrix === Codecs\MatrixCoefficients::Bt601);
var_dump($avifOpts->lossless === false);
var_dump($avifOpts->alpha === Codecs\AlphaMode::Straight);
var_dump($avifOpts->metadata === Codecs\MetadataPolicy::Preserve);
var_dump($avifOpts->threads === null);

$jxlOpts = new Codecs\JxlEncodeOptions();
var_dump($jxlOpts->quality === 90);
var_dump($jxlOpts->effort === 7);
var_dump($jxlOpts->decodingSpeed === 0);
var_dump($jxlOpts->lossless === false);
var_dump($jxlOpts->alpha === Codecs\AlphaMode::Straight);
var_dump($jxlOpts->metadata === Codecs\MetadataPolicy::Preserve);
var_dump($jxlOpts->threads === null);

$decOpts = new Codecs\DecodeOptions();
var_dump($decOpts->maxDimension === 0);
var_dump($decOpts->maxPixels === 0);
var_dump($decOpts->ignoreIcc === false);
var_dump($decOpts->ignoreExif === false);
var_dump($decOpts->ignoreXmp === false);
var_dump($decOpts->keepOrientation === false);
var_dump($decOpts->firstFrameOnly === false);
var_dump($decOpts->matrixOverride === null);
var_dump($decOpts->threads === null);
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
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)

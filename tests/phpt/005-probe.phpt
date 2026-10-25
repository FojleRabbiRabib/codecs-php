--TEST--
Codecs\Codec::probe and probeFile inspect AVIF container metadata
--EXTENSIONS--
codecs
--FILE--
<?php
/*
 * Copyright 2026 Fojle Rabbi (Rabib)
 * SPDX-License-Identifier: Apache-2.0
 */

declare(strict_types=1);

$file = getcwd() . '/vendor-src/libavif-1.4.2/tests/data/colors_text_hdr_srgb.avif';
if (!file_exists($file)) {
    echo "SKIP: fixture not found\n";
    exit(0);
}

$info = Codecs\Codec::probeFile($file);
var_dump($info instanceof Codecs\Info);
var_dump($info->format === Codecs\Format::Avif);
var_dump($info->width === 200);
var_dump($info->height === 200);
var_dump($info->bitDepth === 10);
var_dump($info->alpha === false);
var_dump($info->frameCount === 1);
var_dump($info->animated === false);
var_dump($info->byteSize > 0);
var_dump($info->orientation === Codecs\Orientation::TopLeft);
var_dump($info->cicp instanceof Codecs\Cicp);
var_dump($info->cicp->fullRange === true);

// Empty probe throws CorruptDataException
try {
    Codecs\Codec::probe("");
} catch (Codecs\CorruptDataException $e) {
    echo "empty probe: " . $e->getErrorCode()->name . "\n";
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
empty probe: Bitstream

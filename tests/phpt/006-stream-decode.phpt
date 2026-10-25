--TEST--
Codecs\Codec::decodeStream and decodeFile read streams with bounds checking
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

$fp = fopen($file, 'rb');
$img = Codecs\Codec::decodeStream($fp);
fclose($fp);

var_dump($img->width() === 200);
var_dump($img->height() === 200);

// Empty stream
$mem = fopen('php://memory', 'r+');
try {
    Codecs\Codec::decodeStream($mem);
} catch (Codecs\CorruptDataException $e) {
    echo "empty stream: " . $e->getErrorCode()->name . "\n";
}
fclose($mem);

// Empty file
$tmpEmpty = tempnam(sys_get_temp_dir(), 'empty-avif-');
try {
    Codecs\Codec::decodeFile($tmpEmpty);
} catch (Codecs\CorruptDataException $e) {
    echo "empty file: " . $e->getErrorCode()->name . "\n";
}
@unlink($tmpEmpty);
?>
--EXPECT--
bool(true)
bool(true)
empty stream: Bitstream
empty file: Bitstream

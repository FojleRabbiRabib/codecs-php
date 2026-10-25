--TEST--
Codecs\Codec::decode decodes a self-contained AVIF bitstream
--EXTENSIONS--
codecs
--FILE--
<?php
/*
 * Copyright 2026 Fojle Rabbi (Rabib)
 * SPDX-License-Identifier: Apache-2.0
 */

declare(strict_types=1);

// 305-byte 1x1 white AVIF
$b64 = "AAAAIGZ0eXBhdmlmAAAAAGF2aWZtaWYxbWlhZk1BMUEAAADybWV0YQAAAAAAAAAoaGRscgAAAAAAAAAAcGljdAAAAAAAAAAAAAAAAGxpYmF2aWYAAAAADnBpdG0AAAAAAAEAAAAeaWxvYwAAAABEAAABAAEAAAABAAABGgAAABcAAAAoaWluZgAAAAAAAQAAABppbmZlAgAAAAABAABhdjAxQ29sb3IAAAAAamlwcnAAAABLaXBjbwAAABRpc3BlAAAAAAAAAAEAAAABAAAAEHBpeGkAAAAAAwgICAAAAAxhdjFDgSAAAAAAABNjb2xybmNseAABAA0ABoAAAAAXaXBtYQAAAAAAAAABAAEEAQKDBAAAAB9tZGF0EgAKBzgABhAQ0GkyCh/wP///xAAAr3A=";
$data = base64_decode($b64);

$img = Codecs\Codec::decode($data);
var_dump($img->width());
var_dump($img->height());
var_dump($img->bitDepth());
var_dump($img->alphaMode() === Codecs\AlphaMode::Straight);
var_dump($img->provenance());
var_dump(strlen($img->rgba16()));
var_dump(strlen($img->rgba8()));

$cicp = $img->cicp();
var_dump($cicp instanceof Codecs\Cicp);
var_dump($cicp->fullRange);

// Empty input handling
try {
    Codecs\Codec::decode("");
} catch (Codecs\CorruptDataException $e) {
    echo "empty input: " . $e->getErrorCode()->name . "\n";
}
?>
--EXPECT--
int(1)
int(1)
int(8)
bool(true)
string(4) "avif"
int(8)
int(4)
bool(true)
bool(true)
empty input: Bitstream

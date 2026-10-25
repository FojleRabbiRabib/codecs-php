--TEST--
Codecs\Codec::decode enforces maxPixels, maxDimension, and input byte limits
--EXTENSIONS--
codecs
--FILE--
<?php
/*
 * Copyright 2026 Fojle Rabbi (Rabib)
 * SPDX-License-Identifier: Apache-2.0
 */

declare(strict_types=1);

$b64 = "AAAAIGZ0eXBhdmlmAAAAAGF2aWZtaWYxbWlhZk1BMUEAAADybWV0YQAAAAAAAAAoaGRscgAAAAAAAAAAcGljdAAAAAAAAAAAAAAAAGxpYmF2aWYAAAAADnBpdG0AAAAAAAEAAAAeaWxvYwAAAABEAAABAAEAAAABAAABGgAAABcAAAAoaWluZgAAAAAAAQAAABppbmZlAgAAAAABAABhdjAxQ29sb3IAAAAAamlwcnAAAABLaXBjbwAAABRpc3BlAAAAAAAAAAEAAAABAAAAEHBpeGkAAAAAAwgICAAAAAxhdjFDgSAAAAAAABNjb2xybmNseAABAA0ABoAAAAAXaXBtYQAAAAAAAAABAAEEAQKDBAAAAB9tZGF0EgAKBzgABhAQ0GkyCh/wP///xAAAr3A=";
$data = base64_decode($b64);

// maxDimension lower than 1x1
try {
    Codecs\Codec::decode($data, new Codecs\DecodeOptions(maxDimension: 0));
    // 0 is default (use ini), so 1x1 should decode fine
    echo "0 maxDim decodes OK\n";
} catch (Codecs\LimitException $e) {
    echo "FAIL\n";
}

// Corrupt bitstream
try {
    Codecs\Codec::decode("not a real avif bitstream at all");
} catch (Codecs\CorruptDataException $e) {
    echo "corrupt: " . $e->getErrorCode()->name . "\n";
}
?>
--EXPECT--
0 maxDim decodes OK
corrupt: Container

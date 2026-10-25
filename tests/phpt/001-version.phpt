--TEST--
Codecs\Codec::version() and ::versions() report extension and vendored library versions
--EXTENSIONS--
codecs
--FILE--
<?php
/*
 * Copyright 2026 Fojle Rabbi (Rabib)
 * SPDX-License-Identifier: Apache-2.0
 */

declare(strict_types=1);

var_dump(Codecs\Codec::version() === "0.1.0");

$v = Codecs\Codec::versions();
var_dump(is_array($v));
var_dump($v['codecs_php'] === "0.1.0");
var_dump($v['libavif'] === "1.4.2");
var_dump($v['dav1d'] === "1.5.4");

var_dump(Codecs\Codec::supports(Codecs\Format::Avif));
var_dump(Codecs\Codec::supports(Codecs\Format::JpegXl));
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(false)

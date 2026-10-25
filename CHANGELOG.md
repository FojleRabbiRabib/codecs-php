# Changelog

All notable changes to `codecs-php` are documented in this file.
The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).
Git tags carry the plain extension version (`v0.1.0`); each release pairs it with
pinned vendored upstream codec versions recorded in the tag message,
`php_codecs.h`, and the release notes.

## [Unreleased]

### Added
- In-process AVIF decoding via statically linked libavif 1.4.2 and dav1d 1.5.4 (`Codecs\Codec::decode`, `decodeFile`, `decodeStream`).
- Full typed options and supporting types: `Codecs\Format`, `Codecs\AlphaMode`, `Codecs\ChromaSubsampling`, `Codecs\MatrixCoefficients`, `Codecs\Orientation`, `Codecs\MetadataPolicy`, `Codecs\ErrorCode`, `Codecs\Cicp`, `Codecs\DecodeOptions`, `Codecs\AvifEncodeOptions`, `Codecs\JxlEncodeOptions`, and `Codecs\Info`.
- Typed exception hierarchy: `Codecs\Exception` with `getErrorCode(): ErrorCode`, `DecodeException`, `EncodeException`, `LimitException`, `UnsupportedFormatException`, `CorruptDataException`, and `StreamException`.
- Native `Codecs\Image` object: 16-bit RGBA full-scale storage with straight alpha, `rgba16()` and lossy `rgba8()` accessors, `fromPixels()` with strict length validation, `memoryUsage()`, and idempotent `close()`.
- Resource ceilings enforced at decode time: `codecs.max_dimension`, `codecs.max_pixels`, `codecs.max_input_bytes`, `codecs.max_metadata_bytes`, `codecs.max_frames`, and `codecs.memory_budget`.
- Animated AVIF input fail-closed by default with explicit `firstFrameOnly: true` opt-in.
- Upstream pinning and verification: `tools/pins.txt` (tarball + license SHA-256), `tools/fetch-codecs.sh`, and `tools/build-deps.sh`.
- Raw-pixel fidelity oracle: `tools/dump-pixels.c` and `tests/quality/fidelity.php` asserting byte-identical RGBA16 decode against vendored libavif/dav1d across test fixtures.
- Phpt test suite: `tools/test-phpt.php` running `tests/phpt/001-004`.
- Initial extension scaffold (`config.m4`, `php_codecs.h`, `codecs.c`, `codecs.stub.php`,
  `codecs_arginfo.h`, symbol version script `codecs.map`, and PIE `composer.json` / `package.xml`
  manifests).

<?php

/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2026 Fojle Rabbi (Rabib)
 *
 * @generate-class-entries
 */

namespace Codecs
{
    enum Format: string
    {
        case Avif = 'avif';
        case JpegXl = 'jxl';
        case Heif = 'heif';
    }

    enum AlphaMode: string
    {
        case Straight = 'straight';
        case Premultiplied = 'premultiplied';
    }

    enum ChromaSubsampling: string
    {
        case Yuv444 = '444';
        case Yuv422 = '422';
        case Yuv420 = '420';
        case Yuv400 = '400';
    }

    enum MatrixCoefficients: int
    {
        case Identity = 0;
        case Bt709 = 1;
        case Bt601 = 5;
        case Bt2020Ncl = 9;
        case YcgcoRe = 13;
        case YcgcoRo = 14;
    }

    enum Orientation: int
    {
        case TopLeft = 1;
        case TopRight = 2;
        case BottomRight = 3;
        case BottomLeft = 4;
        case LeftTop = 5;
        case RightTop = 6;
        case RightBottom = 7;
        case LeftBottom = 8;
    }

    enum MetadataPolicy: string
    {
        case Preserve = 'preserve';
        case Strip = 'strip';
    }

    enum ErrorCode: int
    {
        case Ok = 0;
        case Container = 1;
        case Bitstream = 2;
        case Metadata = 3;
        case Limit = 4;
        case Dimension = 5;
        case Pixels = 6;
        case Unsupported = 7;
        case HeifNotCompiled = 8;
        case StreamIo = 9;
        case OutOfMemory = 10;
        case Internal = 11;
    }

    class Exception extends \RuntimeException
    {
        public ErrorCode $errorCode;

        public function getErrorCode(): ErrorCode
        {
        }
    }

    class DecodeException extends Exception
    {
    }

    class EncodeException extends Exception
    {
    }

    class LimitException extends Exception
    {
    }

    class UnsupportedFormatException extends Exception
    {
    }

    class CorruptDataException extends Exception
    {
    }

    class StreamException extends Exception
    {
    }

    interface EncodeOptions
    {
    }

    final readonly class Cicp
    {
        public int $colourPrimaries;
        public int $transferCharacteristics;
        public int $matrixCoefficients;
        public bool $fullRange;

        public function __construct(
            int $colourPrimaries,
            int $transferCharacteristics,
            int $matrixCoefficients,
            bool $fullRange
        ) {
        }
    }

    final readonly class DecodeOptions
    {
        public int $maxDimension;
        public int $maxPixels;
        public bool $ignoreIcc;
        public bool $ignoreExif;
        public bool $ignoreXmp;
        public bool $keepOrientation;
        public bool $firstFrameOnly;
        public ?MatrixCoefficients $matrixOverride;
        public ?int $threads;

        public function __construct(
            int $maxDimension = 0,
            int $maxPixels = 0,
            bool $ignoreIcc = false,
            bool $ignoreExif = false,
            bool $ignoreXmp = false,
            bool $keepOrientation = false,
            bool $firstFrameOnly = false,
            ?MatrixCoefficients $matrixOverride = null,
            ?int $threads = null
        ) {
        }
    }

    final readonly class AvifEncodeOptions implements EncodeOptions
    {
        public int $quality;
        public ?int $qualityAlpha;
        public int $speed;
        public ChromaSubsampling $subsampling;
        public MatrixCoefficients $matrix;
        public bool $lossless;
        public AlphaMode $alpha;
        public MetadataPolicy $metadata;
        public ?int $threads;

        public function __construct(
            int $quality = 60,
            ?int $qualityAlpha = null,
            int $speed = 6,
            ChromaSubsampling $subsampling = ChromaSubsampling::Yuv420,
            MatrixCoefficients $matrix = MatrixCoefficients::Bt601,
            bool $lossless = false,
            AlphaMode $alpha = AlphaMode::Straight,
            MetadataPolicy $metadata = MetadataPolicy::Preserve,
            ?int $threads = null
        ) {
        }
    }

    final readonly class JxlEncodeOptions implements EncodeOptions
    {
        public int $quality;
        public int $effort;
        public int $decodingSpeed;
        public bool $lossless;
        public AlphaMode $alpha;
        public MetadataPolicy $metadata;
        public ?int $threads;

        public function __construct(
            int $quality = 90,
            int $effort = 7,
            int $decodingSpeed = 0,
            bool $lossless = false,
            AlphaMode $alpha = AlphaMode::Straight,
            MetadataPolicy $metadata = MetadataPolicy::Preserve,
            ?int $threads = null
        ) {
        }
    }

    final readonly class Info
    {
        public Format $format;
        public int $width;
        public int $height;
        public int $bitDepth;
        public bool $alpha;
        public Cicp $cicp;
        public ?string $iccProfile;
        public Orientation $orientation;
        public int $frameCount;
        public bool $animated;
        public int $byteSize;
        public array $metadataBlocks;

        /** @param array $metadataBlocks */
        public function __construct(
            Format $format,
            int $width,
            int $height,
            int $bitDepth,
            bool $alpha,
            Cicp $cicp,
            ?string $iccProfile,
            Orientation $orientation,
            int $frameCount,
            bool $animated,
            int $byteSize,
            array $metadataBlocks
        ) {
        }
    }

    final class Image
    {
        public static function fromPixels(int $width, int $height, string $rgba16, AlphaMode $alpha): Image
        {
        }

        public static function fromPng(string $png): Image
        {
        }

        public static function fromGd(object $gd): Image
        {
        }

        public function width(): int
        {
        }

        public function height(): int
        {
        }

        public function bitDepth(): int
        {
        }

        public function alphaMode(): AlphaMode
        {
        }

        public function provenance(): string
        {
        }

        public function cicp(): Cicp
        {
        }

        public function iccProfile(): ?string
        {
        }

        public function exif(): ?string
        {
        }

        public function xmp(): ?string
        {
        }

        public function orientation(): Orientation
        {
        }

        public function memoryUsage(): int
        {
        }

        public function close(): void
        {
        }

        public function toPng(): string
        {
        }

        public function rgba16(): string
        {
        }

        public function rgba8(): string
        {
        }

        private function __clone(): void
        {
        }

        public function __debugInfo(): array
        {
        }
    }

    final class Codec
    {
        public static function decode(string $data, ?DecodeOptions $options = null): Image
        {
        }

        public static function decodeFile(string $path, ?DecodeOptions $options = null): Image
        {
        }

        /** @param resource $stream */
        public static function decodeStream($stream, ?DecodeOptions $options = null): Image
        {
        }

        public static function encode(Image $image, Format $format, ?EncodeOptions $options = null): string
        {
        }

        public static function encodeFile(
            Image $image,
            string $path,
            Format $format,
            ?EncodeOptions $options = null
        ): void {
        }

        /** @param resource $stream */
        public static function encodeStream(
            Image $image,
            $stream,
            Format $format,
            ?EncodeOptions $options = null
        ): void {
        }

        public static function transcode(string $data, Format $format, ?EncodeOptions $options = null): string
        {
        }

        public static function probe(string $data): Info
        {
        }

        public static function probeFile(string $path): Info
        {
        }

        /** @param resource $stream */
        public static function probeStream($stream): Info
        {
        }

        public static function supports(Format $format): bool
        {
        }

        /**
         * @return array
         */
        public static function versions(): array
        {
        }

        public static function version(): string
        {
        }
    }
}

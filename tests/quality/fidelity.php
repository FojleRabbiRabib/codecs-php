<?php

/**
 * Raw-pixel fidelity oracle gate for codecs-php.
 *
 * Copyright 2026 Fojle Rabbi (Rabib)
 * SPDX-License-Identifier: Apache-2.0
 *
 * Decodes test fixtures using Codecs\Codec::decodeFile() and compares rgba16()
 * byte-for-byte against tools/dump-pixels output compiled from the same sources.
 */

declare(strict_types=1);

if (!extension_loaded('codecs')) {
    fwrite(
        STDERR,
        "fidelity: 'codecs' not loaded. Run: php -n -d extension=modules/codecs.so tests/quality/fidelity.php\n"
    );
    exit(2);
}

$dumpTool = __DIR__ . '/../../tools/dump-pixels';
if (!is_executable($dumpTool)) {
    fwrite(STDERR, "fidelity: dump-pixels not found at {$dumpTool}; compile tools/dump-pixels.c first\n");
    exit(2);
}

$fixtures = [
    __DIR__ . '/../../vendor-src/libavif-1.4.2/tests/data/white_1x1.avif',
    __DIR__ . '/../../vendor-src/libavif-1.4.2/tests/data/colors_text_hdr_srgb.avif',
    __DIR__ . '/../../vendor-src/libavif-1.4.2/tests/data/draw_points_idat.avif',
    __DIR__ . '/../fixtures/white_1x1_rgba_lossless.jxl',
    __DIR__ . '/../fixtures/gradient_8x8_rgba_lossless.jxl',
    __DIR__ . '/../fixtures/gradient_64x64_rgb_lossy.jxl',
];

$passed = 0;
$failed = 0;

foreach ($fixtures as $fixture) {
    if (!file_exists($fixture)) {
        fwrite(STDERR, "fidelity: fixture missing: {$fixture}\n");
        $failed++;
        continue;
    }

    $name = basename($fixture);
    $tmpRaw = tempnam(sys_get_temp_dir(), 'codecs-oracle-');

    // Run oracle dump tool
    $cmd = sprintf('%s %s %s 2>&1', escapeshellarg($dumpTool), escapeshellarg($fixture), escapeshellarg($tmpRaw));
    exec($cmd, $out, $code);
    if ($code !== 0) {
        echo "FAIL {$name}: oracle tool exited with {$code}\n";
        $failed++;
        @unlink($tmpRaw);
        continue;
    }

    $oracleBytes = file_get_contents($tmpRaw);
    @unlink($tmpRaw);

    // Run extension decode
    try {
        $img = Codecs\Codec::decodeFile($fixture);
        $extBytes = $img->rgba16();
    } catch (Throwable $e) {
        echo "FAIL {$name}: extension threw: " . $e->getMessage() . "\n";
        $failed++;
        continue;
    }

    if ($extBytes === $oracleBytes) {
        echo "PASS {$name} (byte-identical, " . number_format(strlen($extBytes)) . " bytes)\n";
        $passed++;
    } else {
        echo "FAIL {$name}: byte mismatch (ext=" . strlen($extBytes) . ", oracle=" . strlen($oracleBytes) . ")\n";
        $failed++;
    }
}

printf("fidelity: %d passed, %d failed\n", $passed, $failed);
exit($failed === 0 ? 0 : 1);

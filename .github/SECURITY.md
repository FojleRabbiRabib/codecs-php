# Security Policy

## Supported Versions

Security updates are actively maintained for `codecs-php` on modern PHP versions:

| Version | Supported | PHP Compatibility |
| --- | --- | --- |
| 0.1.x (`develop` / release tags) | :white_check_mark: | PHP 8.3, 8.4, 8.5 |

Security fixes are merged to `develop`, verified across the full test and ceiling battery, and shipped immediately in patch releases.

## Reporting a Vulnerability

**Please do not report security vulnerabilities via public GitHub issues.**

If you discover a security issue in `codecs-php`:
1. Use GitHub's private vulnerability reporting feature on this repository: **Security** tab → **Report a vulnerability**.
2. Provide a description of the vulnerability, affected versions, and a minimal proof-of-concept (POC) script or image payload.
3. We will acknowledge receipt promptly, evaluate the impact, and coordinate a fix timeline before public disclosure.

## In-Scope Vulnerabilities

- **Decompression bombs / resource exhaustion:** Malformed or hostile AVIF/JXL bitstreams that bypass configured limits (`max_dimension`, `max_pixels`, `max_input_bytes`, `max_metadata_bytes`).
- **Memory safety violations:** Buffer overflows, out-of-bounds reads or writes, heap corruption, use-after-free, or unhandled pointer faults in the extension or vendored codec code.
- **Native memory leaks:** Decoder contexts, worker threadpools, or native buffers leaked during execution timeouts or unhandled exceptions.
- **Parser crashes / denial of service:** Truncated or adversarial containers causing native aborts, assertion failures, or infinite loops instead of clean PHP exceptions.

## Out-of-Scope Vulnerabilities

- Normal `memory_limit` exhaustion when user-configured ceilings intentionally permit allocations larger than the host's allocated PHP memory.
- Vulnerabilities originating strictly in the PHP engine core itself.
- Upstream bugs in `libavif` or `libjxl` that reproduce identically using their standalone reference CLI binaries (please report those directly to upstream project maintainers, though notification to us is appreciated).

## Guidance for Processing Untrusted Uploads

When processing untrusted user image uploads in production:
- Keep `codecs.max_dimension` and `codecs.max_pixels` tuned to your application's expected upload geometry.
- Enforce strict upload byte ceilings via `codecs.max_input_bytes`.
- Configure application-level memory budgeting via `codecs.memory_budget` and follow safe worker process recycling patterns under php-fpm.

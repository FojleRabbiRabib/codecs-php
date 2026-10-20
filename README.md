# codecs-php

In-process **AVIF** and **JPEG-XL** image decoding, encoding, and header probing for PHP 8.3+,
backed by statically linked builds of [libavif](https://github.com/AOMediaCodec/libavif) and
[libjxl](https://github.com/libjxl/libjxl).

- **Self-contained single `.so`** — vendored codec libraries linked with a symbol version script
  that exports only `get_module`; no system `libavif`/`libjxl` packages or ImageMagick/GD delegates
  required.
- **In-process** — no `shell_exec` to CLI binaries, no temporary files, and no FFI runtime
  requirement.
- **Multi-ABI** — targets PHP 8.3, 8.4, and 8.5 (PIE `type: php-ext`).

## Requirements

- PHP **8.3**, **8.4**, or **8.5** (NTS or ZTS)
- Linux `x86_64` / `aarch64`

## Building from source

```bash
phpize
./configure --enable-codecs
make -j"$(nproc)"
php -n -d extension=modules/codecs.so --ri codecs
```

## Quick verify

```php
<?php

declare(strict_types=1);

var_dump(Codecs\Codec::version());
```

## License

Apache License 2.0 — see [LICENSE](LICENSE) and [NOTICE](NOTICE).

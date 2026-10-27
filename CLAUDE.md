# CLAUDE.md — codecs-php

Project instructions for AI assistants working in this repository.

## What this project is

A standalone native PHP extension providing in-process modern image codec decoding and encoding
(**AVIF** via `libavif` BSD-2-Clause and **JPEG-XL** via `libjxl` BSD-3-Clause, with **HEIC/HEIF**
following as an optional module) statically linked into a self-contained PHP extension (`codecs.so`).
PECL/PIE-canonical root layout.

## Build

```bash
phpize && ./configure --enable-codecs
make -j"$(nproc)"
```

Vendored codec sources in `vendor-src/` are **fetched and SHA-256 verified by the build scripts,
never committed** — `vendor-src/` is gitignored because it is reproducible from the pinned hashes.

## Layout

| Path | Role |
|---|---|
| `config.m4` | Toolchain checks, hardening probes, sources, link hardening |
| `php_codecs.h` | Module header and version constants |
| `codecs_internal.h` | Internal status codes, resource ceilings, and shared declarations |
| `codecs.c` | Module entry, lifecycle, and `Codecs` class registration |
| `codecs.stub.php` | Public surface; `codecs_arginfo.h` is generated from it |
| `codecs_arginfo.h` | Generated arginfo header (never hand-edit or run `clang-format` on it) |
| `codecs.map` / `codecs.exp` | Linker version script / export list (`get_module` only) |
| `package.xml` | PECL/PIE package manifest |
| `composer.json` | PIE (`type: php-ext`) and dev-tooling manifest |

Files in the repository root that are planning or research records stay **untracked and never
committed**. Product manuals live in `docs/` when shipped.

## Engineering rules

- **PHP 8.3+ only** — no PHP 5/7-era extension patterns. Multi-ABI order: **8.3 first**, then 8.4
  and 8.5 only after 8.3 passes.
- **Zero legacy/compat scaffolding** — when something changes, update every caller.
- **Never weaken a gate** to make a failure go green. Every red gate is a real defect.
- **`-Wall -Wextra -Werror`** on our own sources.
- **No C++ exception may cross the C boundary** (libjxl is C++ internally behind its C API).
- **Symbol isolation is load-bearing** — `codecs.map` must export only `get_module`.
- **Report real memory** (`/proc/self/status` VmRSS), never Zend MM alone, for native pixel/codec
  allocations.
- **Fidelity gate:** decoded/encoded output must match the upstream CLI binaries (`avifdec`/
  `avifenc`, `djxl`/`cjxl`) built from the same vendored tree, alongside the adversarial bitstream
  safety suite.

## Licence and attribution

Apache-2.0. Every source file carries an `SPDX-License-Identifier: Apache-2.0` header. `NOTICE`
credits `codecs-php` and the vendored upstream libraries (`libavif`, `dav1d`, `libjxl`, `highway`);
it must be preserved by anyone redistributing this work.

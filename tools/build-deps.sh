#!/usr/bin/env bash
# tools/build-deps.sh — compile vendored static libraries with PIC for linking into codecs.so.
#
# Copyright 2026 Fojle Rabbi (Rabib)
# SPDX-License-Identifier: Apache-2.0
#
# Builds pinned dav1d and libavif from vendor-src/ into build/vendor-install/.
# Both produce static archives (-fPIC) ready to be linked into the PHP extension.

set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
V="$ROOT/vendor-src"
B="$ROOT/vendor-build"
INSTALL="$ROOT/vendor-build/install"

# Ensure sources are present and verified
"$ROOT/tools/fetch-codecs.sh"

DAV1D_SRC="$(find "$V" -mindepth 1 -maxdepth 1 -type d -name "dav1d-*" | head -1)"
AVIF_SRC="$(find "$V" -mindepth 1 -maxdepth 1 -type d -name "libavif-*" | head -1)"

if [ -z "$DAV1D_SRC" ] || [ -z "$AVIF_SRC" ]; then
	echo "FAIL: vendored sources missing in $V" >&2
	exit 1
fi

mkdir -p "$B" "$INSTALL"

# 1. dav1d static
echo "=== building dav1d static ===" >&2
DAV1D_B="$B/dav1d"
if [ ! -f "$INSTALL/lib/libdav1d.a" ]; then
	rm -rf "$DAV1D_B"
	# Detect nasm for assembly; fall back to C implementation cleanly if absent
	ENABLE_ASM="true"
	if ! command -v nasm >/dev/null 2>&1; then
		ENABLE_ASM="false"
		echo "note: nasm not found, building dav1d with enable_asm=false" >&2
	fi
	meson setup "$DAV1D_B" "$DAV1D_SRC" \
		--prefix="$INSTALL" \
		--libdir="lib" \
		--default-library=static \
		--buildtype=release \
		-Db_pie=true \
		-Denable_tools=false \
		-Denable_tests=false \
		-Denable_asm="$ENABLE_ASM"
	ninja -C "$DAV1D_B" install
fi

# 2. libavif static linking against dav1d
echo "=== building libavif static ===" >&2
AVIF_B="$B/avif"
if [ ! -f "$INSTALL/lib/libavif.a" ]; then
	rm -rf "$AVIF_B"
	PKG_CONFIG_PATH="$INSTALL/lib/pkgconfig:${PKG_CONFIG_PATH:-}" \
	cmake -S "$AVIF_SRC" -B "$AVIF_B" \
		-DCMAKE_INSTALL_PREFIX="$INSTALL" \
		-DCMAKE_INSTALL_LIBDIR="lib" \
		-DBUILD_SHARED_LIBS=OFF \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
		-DAVIF_CODEC_DAV1D=SYSTEM \
		-DAVIF_LIBYUV=OFF \
		-DAVIF_BUILD_APPS=OFF \
		-DAVIF_BUILD_TESTS=OFF \
		-DAVIF_ENABLE_WERROR=OFF
	cmake --build "$AVIF_B" -j"$(nproc)"
	cmake --install "$AVIF_B"
fi

echo "ok: vendored static libraries ready in $INSTALL" >&2
ls -lh "$INSTALL/lib/"*.a >&2

# 3. Build oracle dump tool
echo "=== building dump-pixels oracle ===" >&2
gcc -O2 -Wall -Wextra "$ROOT/tools/dump-pixels.c" \
	-I"$INSTALL/include" \
	"$INSTALL/lib/libavif.a" \
	"$INSTALL/lib/libdav1d.a" \
	-lm -lpthread -ldl \
	-o "$ROOT/tools/dump-pixels"
echo "ok: oracle tool ready at $ROOT/tools/dump-pixels" >&2

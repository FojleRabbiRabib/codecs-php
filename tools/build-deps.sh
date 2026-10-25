#!/usr/bin/env bash
# tools/build-deps.sh — compile vendored static libraries with PIC for linking into codecs.so.
#
# Copyright 2026 Fojle Rabbi (Rabib)
# SPDX-License-Identifier: Apache-2.0
#
# Builds pinned dav1d, libavif, and libjxl (with highway, brotli, skcms) from
# vendor-src/ into vendor-build/install/. All produce static archives (-fPIC)
# ready to be linked into the PHP extension. Source directories are resolved
# deterministically from tools/pins.txt, and every install is stamped with the
# pin version so a bump always rebuilds and never silently keeps a superseded
# library.

set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
V="$ROOT/vendor-src"
B="$ROOT/vendor-build"
INSTALL="$ROOT/vendor-build/install"
PINS="$ROOT/tools/pins.txt"

# pin_version <name> — resolve the pinned version from tools/pins.txt.
pin_version() {
	awk -v n="$1" '$1 == n { print $2; found=1 } END { if (!found) exit 1 }' "$PINS"
}

# pin_source <name> — fail-closed source dir for a pin: version must exist in
# pins.txt and the fetched, verified tree must be present in vendor-src/.
pin_source() {
	local name="$1" ver dir
	ver="$(pin_version "$name")" || {
		echo "FAIL: no '$name' row in $PINS" >&2
		exit 1
	}
	dir="$V/$name-$ver"
	if [ ! -d "$dir" ]; then
		echo "FAIL: $name-$ver missing in $V; run tools/fetch-codecs.sh" >&2
		exit 1
	fi
	printf '%s' "$dir"
}

# Ensure sources are present and verified
"$ROOT/tools/fetch-codecs.sh"

DAV1D_SRC="$(pin_source dav1d)"
AVIF_SRC="$(pin_source libavif)"
JXL_SRC="$(pin_source libjxl)"
HWY_SRC="$(pin_source highway)"
BROTLI_SRC="$(pin_source brotli)"
SKCMS_SRC="$(pin_source skcms)"

DAV1D_VER="$(pin_version dav1d)"
AVIF_VER="$(pin_version libavif)"
JXL_VER="$(pin_version libjxl)"

# rebuild_if_stale <archive> <stamp>: returns 0 (rebuild needed) when either the
# installed archive or the pin-version stamp is missing (cache keyed to the pin version).
rebuild_if_stale() {
	[ ! -f "$1" ] || [ ! -f "$2" ]
}

mkdir -p "$B" "$INSTALL"

# 1. dav1d static
echo "=== building dav1d static ===" >&2
DAV1D_B="$B/dav1d"
DAV1D_STAMP="$INSTALL/lib/.codecs-stamp-dav1d-$DAV1D_VER"
if rebuild_if_stale "$INSTALL/lib/libdav1d.a" "$DAV1D_STAMP"; then
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
	touch "$DAV1D_STAMP"
fi

# 2. libavif static linking against dav1d
echo "=== building libavif static ===" >&2
AVIF_B="$B/avif"
AVIF_STAMP="$INSTALL/lib/.codecs-stamp-libavif-$AVIF_VER"
if rebuild_if_stale "$INSTALL/lib/libavif.a" "$AVIF_STAMP"; then
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
	touch "$AVIF_STAMP"
fi

# 3. libjxl static linking against highway, brotli, skcms
echo "=== building libjxl static ===" >&2
mkdir -p "$JXL_SRC/third_party"
rm -rf "$JXL_SRC/third_party/highway" "$JXL_SRC/third_party/brotli" "$JXL_SRC/third_party/skcms"
ln -sfn "$HWY_SRC" "$JXL_SRC/third_party/highway"
ln -sfn "$BROTLI_SRC" "$JXL_SRC/third_party/brotli"
ln -sfn "$SKCMS_SRC" "$JXL_SRC/third_party/skcms"

JXL_B="$B/jxl"
JXL_STAMP="$INSTALL/lib/.codecs-stamp-libjxl-$JXL_VER"
if rebuild_if_stale "$INSTALL/lib/libjxl.a" "$JXL_STAMP"; then
	rm -rf "$JXL_B"
	cmake -S "$JXL_SRC" -B "$JXL_B" \
		-DCMAKE_INSTALL_PREFIX="$INSTALL" \
		-DCMAKE_INSTALL_LIBDIR="lib" \
		-DBUILD_SHARED_LIBS=OFF \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
		-DBUILD_TESTING=OFF \
		-DJPEGXL_ENABLE_TOOLS=OFF \
		-DJPEGXL_ENABLE_DOXYGEN=OFF \
		-DJPEGXL_ENABLE_MANPAGES=OFF \
		-DJPEGXL_ENABLE_BENCHMARK=OFF \
		-DJPEGXL_ENABLE_EXAMPLES=OFF \
		-DJPEGXL_ENABLE_JNI=OFF \
		-DJPEGXL_ENABLE_SJPEG=OFF \
		-DJPEGXL_ENABLE_OPENEXR=OFF \
		-DJPEGXL_ENABLE_SKCMS=ON \
		-DJPEGXL_FORCE_SYSTEM_BROTLI=OFF \
		-DJPEGXL_FORCE_SYSTEM_HWY=OFF \
		-DJPEGXL_ENABLE_TRANSCODE_JPEG=OFF
	cmake --build "$JXL_B" -j"$(nproc)"
	cmake --install "$JXL_B"
	touch "$JXL_STAMP"
fi

echo "ok: vendored static libraries ready in $INSTALL" >&2
ls -lh "$INSTALL/lib/"*.a >&2

# 4. Build oracle dump tool
echo "=== building dump-pixels oracle ===" >&2
gcc -O2 -Wall -Wextra "$ROOT/tools/dump-pixels.c" \
	-I"$INSTALL/include" \
	"$INSTALL/lib/libavif.a" \
	"$INSTALL/lib/libdav1d.a" \
	-lm -lpthread -ldl \
	-o "$ROOT/tools/dump-pixels"
echo "ok: oracle tool ready at $ROOT/tools/dump-pixels" >&2

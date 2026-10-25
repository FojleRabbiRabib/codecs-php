#!/usr/bin/env bash
# tools/fetch-codecs.sh — the single fetch+verify path for pinned codec sources.
#
# Copyright 2026 Fojle Rabbi (Rabib)
# SPDX-License-Identifier: Apache-2.0
#
# Reads tools/pins.txt, downloads missing tarballs to /tmp/codecs-cache (or
# $CODECS_CACHE), verifies both the archive SHA-256 and the extracted license
# SHA-256, and extracts each into vendor-src/.

set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PINS="$ROOT/tools/pins.txt"
CACHE="${CODECS_CACHE:-/tmp/codecs-cache}"
V="$ROOT/vendor-src"

calc_sha256() {
	if command -v sha256sum >/dev/null 2>&1; then
		sha256sum "$1" | awk '{print $1}'
	elif command -v shasum >/dev/null 2>&1; then
		shasum -a 256 "$1" | awk '{print $1}'
	else
		echo "FAIL: neither sha256sum nor shasum found" >&2
		exit 1
	fi
}

mkdir -p "$CACHE" "$V"

while IFS=' ' read -r name version archive_sha lic_rel lic_sha url || [ -n "$name" ]; do
	[ -z "$name" ] && continue
	[[ "$name" =~ ^# ]] && continue

	target_dir="$V/$name-$version"
	tarball="$CACHE/$name-$version.tar.gz"

	if [ -d "$target_dir" ]; then
		# Target directory already exists; verify its license hash
		if [ -f "$target_dir/$lic_rel" ]; then
			actual_lic="$(calc_sha256 "$target_dir/$lic_rel")"
			if [ "$actual_lic" = "$lic_sha" ]; then
				echo "ok: $name $version (already verified in vendor-src)" >&2
				continue
			fi
		fi
		echo "re-extracting: $name $version" >&2
		rm -rf "$target_dir"
	fi

	echo "fetching: $name $version from $url" >&2
	if [ ! -f "$tarball" ]; then
		curl -fsSL --retry 3 -o "$tarball" "$url"
	fi

	actual_archive="$(calc_sha256 "$tarball")"
	if [ "$actual_archive" != "$archive_sha" ]; then
		echo "FAIL: archive SHA256 mismatch for $tarball (expected $archive_sha, got $actual_archive)" >&2
		rm -f "$tarball"
		exit 1
	fi

	tmp_extract="$V/.tmp-$name-$version"
	rm -rf "$tmp_extract"
	mkdir -p "$tmp_extract"
	tar -xzf "$tarball" -C "$tmp_extract"

	# Normalise top-level dir (tarballs may extract to name-version or name-tag)
	extracted_dir="$(find "$tmp_extract" -mindepth 1 -maxdepth 1 -type d | head -1)"
	if [ -z "$extracted_dir" ]; then
		echo "FAIL: no directory found in $tarball" >&2
		rm -rf "$tmp_extract"
		exit 1
	fi

	actual_lic="$(calc_sha256 "$extracted_dir/$lic_rel")"
	if [ "$actual_lic" != "$lic_sha" ]; then
		echo "FAIL: license SHA256 mismatch for $name ($lic_rel) (expected $lic_sha, got $actual_lic)" >&2
		rm -rf "$tmp_extract"
		exit 1
	fi

	mv "$extracted_dir" "$target_dir"
	rm -rf "$tmp_extract"
	echo "verified: $name $version" >&2
done < "$PINS"

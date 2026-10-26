dnl SPDX-License-Identifier: Apache-2.0
dnl Copyright 2026 Fojle Rabbi (Rabib)
dnl
dnl codecs-php extension configuration.

PHP_ARG_ENABLE([codecs],
  [whether to enable the codecs extension],
  [AS_HELP_STRING([--enable-codecs],
    [Enable codecs (in-process AVIF and JPEG-XL image codecs)])],
  [no])

if test "$PHP_CODECS" != "no"; then

  CODECS_VENDOR_INSTALL="${CODECS_VENDOR_INSTALL:-$abs_srcdir/vendor-build/install}"
  if test ! -f "$CODECS_VENDOR_INSTALL/lib/libavif.a"; then
    AC_MSG_ERROR([vendored libavif.a missing at $CODECS_VENDOR_INSTALL/lib/libavif.a; run tools/build-deps.sh first])
  fi

  PHP_ADD_INCLUDE([$abs_srcdir])
  PHP_ADD_INCLUDE([$CODECS_VENDOR_INSTALL/include])

  CODECS_HARDEN_CFLAGS="-fstack-protector-strong -fvisibility=hidden"

  AC_MSG_CHECKING([whether the C toolchain supports -fstack-clash-protection])
  codecs_save_CFLAGS="$CFLAGS"
  CFLAGS="$CFLAGS -Werror -fstack-clash-protection"
  AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM([], [[return 0;]])],
    [codecs_clash=yes], [codecs_clash=no])
  CFLAGS="$codecs_save_CFLAGS"
  AC_MSG_RESULT([$codecs_clash])
  if test "$codecs_clash" = "yes"; then
    CODECS_HARDEN_CFLAGS="$CODECS_HARDEN_CFLAGS -fstack-clash-protection"
  fi

  AC_MSG_CHECKING([whether the C toolchain supports _FORTIFY_SOURCE=3])
  codecs_save_CFLAGS="$CFLAGS"
  CFLAGS="$CFLAGS -O2 -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3"
  AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM([[#include <string.h>]],
      [[char b[16]; strcpy(b, "abc"); return (int)strlen(b);]])],
    [CODECS_FORTIFY=3], [CODECS_FORTIFY=2])
  CFLAGS="$codecs_save_CFLAGS"
  AC_MSG_RESULT([$CODECS_FORTIFY])
  CODECS_HARDEN_CFLAGS="$CODECS_HARDEN_CFLAGS -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=$CODECS_FORTIFY"

  case "$host_cpu:$host_os" in
    x86_64:darwin*|amd64:darwin*) ;;
    x86_64:*|amd64:*) CODECS_HARDEN_CFLAGS="$CODECS_HARDEN_CFLAGS -fcf-protection=full" ;;
  esac

  PHP_NEW_EXTENSION([codecs], [codecs.c codecs_exception.c codecs_image.c codecs_options.c codecs_avif.c codecs_jxl.c], [$ext_shared],,
    [-Wall -Wextra -Werror -Wformat -Wformat-security $CODECS_HARDEN_CFLAGS])

  CODECS_STATIC_LIBS="$CODECS_VENDOR_INSTALL/lib/libavif.a"
  if test -f "$CODECS_VENDOR_INSTALL/lib/libdav1d.a"; then
    CODECS_STATIC_LIBS="$CODECS_STATIC_LIBS $CODECS_VENDOR_INSTALL/lib/libdav1d.a"
  fi
  for jxl_lib in libjxl.a libjxl_cms.a libjxl_threads.a libhwy.a libbrotlienc.a libbrotlidec.a libbrotlicommon.a; do
    if test -f "$CODECS_VENDOR_INSTALL/lib/$jxl_lib"; then
      CODECS_STATIC_LIBS="$CODECS_STATIC_LIBS $CODECS_VENDOR_INSTALL/lib/$jxl_lib"
    fi
  done

  case "$host_os" in
    darwin*)
      CODECS_EXPORTS_LIST="$abs_srcdir/codecs.exp"
      if test ! -f "$CODECS_EXPORTS_LIST"; then
        AC_MSG_ERROR([codecs.exp not found at $CODECS_EXPORTS_LIST; it is part of the extension sources])
      fi
      CODECS_SHARED_LIBADD="$CODECS_STATIC_LIBS -lm -lpthread -lstdc++ $CODECS_SHARED_LIBADD -Wl,-dead_strip -Wl,-bind_at_load -Wl,-undefined,dynamic_lookup -Wl,-exported_symbols_list,$CODECS_EXPORTS_LIST"
      ;;
    *)
      CODECS_LINK_SCRIPT="$abs_srcdir/codecs.map"
      if test ! -f "$CODECS_LINK_SCRIPT"; then
        AC_MSG_ERROR([codecs.map not found at $CODECS_LINK_SCRIPT; it is part of the extension sources])
      fi
      CODECS_SHARED_LIBADD="$CODECS_STATIC_LIBS -lm -lpthread -ldl -lstdc++ $CODECS_SHARED_LIBADD -Wl,--version-script=$CODECS_LINK_SCRIPT -Wl,--gc-sections -Wl,-z,relro,-z,now,-z,noexecstack"
      ;;
  esac

  PHP_SUBST([CODECS_SHARED_LIBADD])
fi

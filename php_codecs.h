/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2026 Fojle Rabbi (Rabib)
 *
 * codecs-php — in-process modern image codecs (AVIF, JPEG-XL) for PHP.
 */

#ifndef PHP_CODECS_H
#define PHP_CODECS_H

#if PHP_VERSION_ID < 80300
#error "codecs requires PHP 8.3 or newer"
#endif

extern zend_module_entry codecs_module_entry;
#define phpext_codecs_ptr &codecs_module_entry

#define PHP_CODECS_VERSION "0.1.0"
#define PHP_CODECS_EXTNAME "codecs"

#if defined(ZTS) && defined(COMPILE_DL_CODECS)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#endif /* PHP_CODECS_H */

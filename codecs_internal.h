/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2026 Fojle Rabbi (Rabib)
 *
 * codecs-php — internal declarations, status codes, and resource ceilings.
 */

#ifndef CODECS_INTERNAL_H
#define CODECS_INTERNAL_H

#include "php.h"

#define CODECS_DEFAULT_MAX_DIMENSION 16384
#define CODECS_DEFAULT_MAX_PIXELS (64ULL * 1024ULL * 1024ULL)

extern zend_class_entry *codecs_exception_ce;
extern zend_class_entry *codecs_codec_ce;

#endif /* CODECS_INTERNAL_H */

/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2026 Fojle Rabbi (Rabib)
 *
 * codecs-php — module lifecycle, class registration, and extension info.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "ext/spl/spl_exceptions.h"
#include "ext/standard/info.h"
#include "php_codecs.h"
#include "codecs_internal.h"
#include "codecs_arginfo.h"

zend_class_entry *codecs_exception_ce = NULL;
zend_class_entry *codecs_codec_ce = NULL;

ZEND_METHOD(Codecs_Codec, version)
{
	ZEND_PARSE_PARAMETERS_NONE();

	RETURN_STRING(PHP_CODECS_VERSION);
}

PHP_MINIT_FUNCTION(codecs)
{
	(void)type;
	(void)module_number;

	codecs_exception_ce = register_class_Codecs_Exception(spl_ce_RuntimeException);
	codecs_codec_ce = register_class_Codecs_Codec();

	return SUCCESS;
}

PHP_MINFO_FUNCTION(codecs)
{
	(void)zend_module;

	php_info_print_table_start();
	php_info_print_table_header(2, "codecs support", "enabled");
	php_info_print_table_row(2, "Extension version", PHP_CODECS_VERSION);
	php_info_print_table_end();
}

zend_module_entry codecs_module_entry = {
	STANDARD_MODULE_HEADER,
	PHP_CODECS_EXTNAME,
	NULL,
	PHP_MINIT(codecs),
	NULL,
	NULL,
	NULL,
	PHP_MINFO(codecs),
	PHP_CODECS_VERSION,
	STANDARD_MODULE_PROPERTIES};

#ifdef COMPILE_DL_CODECS
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(codecs)
#endif

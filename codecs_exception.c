/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2026 Fojle Rabbi (Rabib)
 *
 * codecs-php — exception handling and error code assignment.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "zend_exceptions.h"
#include "codecs_internal.h"

PHP_METHOD(Codecs_Exception, getErrorCode);

void codecs_throw(zend_class_entry *ce, enum codecs_error_code code, const char *format, ...)
{
	va_list args;
	char *message;

	va_start(args, format);
	vspprintf(&message, 0, format, args);
	va_end(args);

	zval ex;
	object_init_ex(&ex, ce ? ce : codecs_exception_ce);

	/* Set message property */
	zend_update_property_string(ce ? ce : codecs_exception_ce, Z_OBJ(ex), "message", sizeof("message") - 1, message);
	efree(message);

	/* Set errorCode as ErrorCode enum case */
	if (codecs_error_code_ce) {
		zend_object *case_obj = NULL;
		if (zend_enum_get_case_by_value(&case_obj, codecs_error_code_ce, code, NULL, false) == SUCCESS && case_obj) {
			zval case_zv;
			ZVAL_OBJ(&case_zv, case_obj);
			zend_update_property(ce ? ce : codecs_exception_ce, Z_OBJ(ex), "errorCode", sizeof("errorCode") - 1, &case_zv);
		}
	}

	zend_throw_exception_object(&ex);
}

ZEND_METHOD(Codecs_Exception, getErrorCode)
{
	ZEND_PARSE_PARAMETERS_NONE();

	zval *code_zv = zend_read_property(codecs_exception_ce, Z_OBJ_P(ZEND_THIS), "errorCode", sizeof("errorCode") - 1, 1, return_value);
	if (code_zv && !Z_ISUNDEF_P(code_zv)) {
		RETURN_COPY(code_zv);
	}
	RETURN_NULL();
}

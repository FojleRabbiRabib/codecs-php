/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 7eeaf7a877bde4e458a656d3c373676507d80c1a */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Codecs_Codec_version, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(Codecs_Codec, version);


static const zend_function_entry class_Codecs_Exception_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Codecs_Codec_methods[] = {
	ZEND_ME(Codecs_Codec, version, arginfo_class_Codecs_Codec_version, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Codecs_Exception(zend_class_entry *class_entry_RuntimeException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Codecs", "Exception", class_Codecs_Exception_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_RuntimeException);

	return class_entry;
}

static zend_class_entry *register_class_Codecs_Codec(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Codecs", "Codec", class_Codecs_Codec_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL;

	return class_entry;
}

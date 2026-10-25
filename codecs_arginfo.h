/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 0b1feacb9bff2a00aca8ac96f9fe51b961eba9b0 */

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Codecs_Exception_getErrorCode, 0, 0, Codecs\\ErrorCode, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Codecs_Cicp___construct, 0, 0, 4)
	ZEND_ARG_TYPE_INFO(0, colourPrimaries, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, transferCharacteristics, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, matrixCoefficients, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, fullRange, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Codecs_DecodeOptions___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, maxDimension, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, maxPixels, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, ignoreIcc, _IS_BOOL, 0, "false")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, ignoreExif, _IS_BOOL, 0, "false")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, ignoreXmp, _IS_BOOL, 0, "false")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, keepOrientation, _IS_BOOL, 0, "false")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, firstFrameOnly, _IS_BOOL, 0, "false")
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, matrixOverride, Codecs\\MatrixCoefficients, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, threads, IS_LONG, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Codecs_AvifEncodeOptions___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, quality, IS_LONG, 0, "60")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, qualityAlpha, IS_LONG, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, speed, IS_LONG, 0, "6")
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, subsampling, Codecs\\ChromaSubsampling, 0, "Codecs\\ChromaSubsampling::Yuv420")
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, matrix, Codecs\\MatrixCoefficients, 0, "Codecs\\MatrixCoefficients::Bt601")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, lossless, _IS_BOOL, 0, "false")
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, alpha, Codecs\\AlphaMode, 0, "Codecs\\AlphaMode::Straight")
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, metadata, Codecs\\MetadataPolicy, 0, "Codecs\\MetadataPolicy::Preserve")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, threads, IS_LONG, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Codecs_JxlEncodeOptions___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, quality, IS_LONG, 0, "90")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, effort, IS_LONG, 0, "7")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, decodingSpeed, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, lossless, _IS_BOOL, 0, "false")
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, alpha, Codecs\\AlphaMode, 0, "Codecs\\AlphaMode::Straight")
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, metadata, Codecs\\MetadataPolicy, 0, "Codecs\\MetadataPolicy::Preserve")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, threads, IS_LONG, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Codecs_Info___construct, 0, 0, 12)
	ZEND_ARG_OBJ_INFO(0, format, Codecs\\Format, 0)
	ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, bitDepth, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, alpha, _IS_BOOL, 0)
	ZEND_ARG_OBJ_INFO(0, cicp, Codecs\\Cicp, 0)
	ZEND_ARG_TYPE_INFO(0, iccProfile, IS_STRING, 1)
	ZEND_ARG_OBJ_INFO(0, orientation, Codecs\\Orientation, 0)
	ZEND_ARG_TYPE_INFO(0, frameCount, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, animated, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, byteSize, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, metadataBlocks, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Codecs_Image_fromPixels, 0, 4, Codecs\\Image, 0)
	ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, rgba16, IS_STRING, 0)
	ZEND_ARG_OBJ_INFO(0, alpha, Codecs\\AlphaMode, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Codecs_Image_fromPng, 0, 1, Codecs\\Image, 0)
	ZEND_ARG_TYPE_INFO(0, png, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Codecs_Image_fromGd, 0, 1, Codecs\\Image, 0)
	ZEND_ARG_TYPE_INFO(0, gd, IS_OBJECT, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Codecs_Image_width, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Codecs_Image_height arginfo_class_Codecs_Image_width

#define arginfo_class_Codecs_Image_bitDepth arginfo_class_Codecs_Image_width

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Codecs_Image_alphaMode, 0, 0, Codecs\\AlphaMode, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Codecs_Image_provenance, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Codecs_Image_cicp, 0, 0, Codecs\\Cicp, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Codecs_Image_iccProfile, 0, 0, IS_STRING, 1)
ZEND_END_ARG_INFO()

#define arginfo_class_Codecs_Image_exif arginfo_class_Codecs_Image_iccProfile

#define arginfo_class_Codecs_Image_xmp arginfo_class_Codecs_Image_iccProfile

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Codecs_Image_orientation, 0, 0, Codecs\\Orientation, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Codecs_Image_memoryUsage arginfo_class_Codecs_Image_width

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Codecs_Image_close, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Codecs_Image_toPng arginfo_class_Codecs_Image_provenance

#define arginfo_class_Codecs_Image_rgba16 arginfo_class_Codecs_Image_provenance

#define arginfo_class_Codecs_Image_rgba8 arginfo_class_Codecs_Image_provenance

#define arginfo_class_Codecs_Image___clone arginfo_class_Codecs_Image_close

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Codecs_Image___debugInfo, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Codecs_Codec_decode, 0, 1, Codecs\\Image, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, options, Codecs\\DecodeOptions, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Codecs_Codec_decodeFile, 0, 1, Codecs\\Image, 0)
	ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, options, Codecs\\DecodeOptions, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Codecs_Codec_decodeStream, 0, 1, Codecs\\Image, 0)
	ZEND_ARG_INFO(0, stream)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, options, Codecs\\DecodeOptions, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Codecs_Codec_encode, 0, 2, IS_STRING, 0)
	ZEND_ARG_OBJ_INFO(0, image, Codecs\\Image, 0)
	ZEND_ARG_OBJ_INFO(0, format, Codecs\\Format, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, options, Codecs\\EncodeOptions, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Codecs_Codec_encodeFile, 0, 3, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO(0, image, Codecs\\Image, 0)
	ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
	ZEND_ARG_OBJ_INFO(0, format, Codecs\\Format, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, options, Codecs\\EncodeOptions, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Codecs_Codec_encodeStream, 0, 3, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO(0, image, Codecs\\Image, 0)
	ZEND_ARG_INFO(0, stream)
	ZEND_ARG_OBJ_INFO(0, format, Codecs\\Format, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, options, Codecs\\EncodeOptions, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Codecs_Codec_transcode, 0, 2, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	ZEND_ARG_OBJ_INFO(0, format, Codecs\\Format, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, options, Codecs\\EncodeOptions, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Codecs_Codec_probe, 0, 1, Codecs\\Info, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Codecs_Codec_probeFile, 0, 1, Codecs\\Info, 0)
	ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Codecs_Codec_probeStream, 0, 1, Codecs\\Info, 0)
	ZEND_ARG_INFO(0, stream)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Codecs_Codec_supports, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_OBJ_INFO(0, format, Codecs\\Format, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Codecs_Codec_versions arginfo_class_Codecs_Image___debugInfo

#define arginfo_class_Codecs_Codec_version arginfo_class_Codecs_Image_provenance


ZEND_METHOD(Codecs_Exception, getErrorCode);
ZEND_METHOD(Codecs_Cicp, __construct);
ZEND_METHOD(Codecs_DecodeOptions, __construct);
ZEND_METHOD(Codecs_AvifEncodeOptions, __construct);
ZEND_METHOD(Codecs_JxlEncodeOptions, __construct);
ZEND_METHOD(Codecs_Info, __construct);
ZEND_METHOD(Codecs_Image, fromPixels);
ZEND_METHOD(Codecs_Image, fromPng);
ZEND_METHOD(Codecs_Image, fromGd);
ZEND_METHOD(Codecs_Image, width);
ZEND_METHOD(Codecs_Image, height);
ZEND_METHOD(Codecs_Image, bitDepth);
ZEND_METHOD(Codecs_Image, alphaMode);
ZEND_METHOD(Codecs_Image, provenance);
ZEND_METHOD(Codecs_Image, cicp);
ZEND_METHOD(Codecs_Image, iccProfile);
ZEND_METHOD(Codecs_Image, exif);
ZEND_METHOD(Codecs_Image, xmp);
ZEND_METHOD(Codecs_Image, orientation);
ZEND_METHOD(Codecs_Image, memoryUsage);
ZEND_METHOD(Codecs_Image, close);
ZEND_METHOD(Codecs_Image, toPng);
ZEND_METHOD(Codecs_Image, rgba16);
ZEND_METHOD(Codecs_Image, rgba8);
ZEND_METHOD(Codecs_Image, __clone);
ZEND_METHOD(Codecs_Image, __debugInfo);
ZEND_METHOD(Codecs_Codec, decode);
ZEND_METHOD(Codecs_Codec, decodeFile);
ZEND_METHOD(Codecs_Codec, decodeStream);
ZEND_METHOD(Codecs_Codec, encode);
ZEND_METHOD(Codecs_Codec, encodeFile);
ZEND_METHOD(Codecs_Codec, encodeStream);
ZEND_METHOD(Codecs_Codec, transcode);
ZEND_METHOD(Codecs_Codec, probe);
ZEND_METHOD(Codecs_Codec, probeFile);
ZEND_METHOD(Codecs_Codec, probeStream);
ZEND_METHOD(Codecs_Codec, supports);
ZEND_METHOD(Codecs_Codec, versions);
ZEND_METHOD(Codecs_Codec, version);


static const zend_function_entry class_Codecs_Format_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Codecs_AlphaMode_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Codecs_ChromaSubsampling_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Codecs_MatrixCoefficients_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Codecs_Orientation_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Codecs_MetadataPolicy_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Codecs_ErrorCode_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Codecs_Exception_methods[] = {
	ZEND_ME(Codecs_Exception, getErrorCode, arginfo_class_Codecs_Exception_getErrorCode, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


static const zend_function_entry class_Codecs_DecodeException_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Codecs_EncodeException_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Codecs_LimitException_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Codecs_UnsupportedFormatException_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Codecs_CorruptDataException_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Codecs_StreamException_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Codecs_EncodeOptions_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Codecs_Cicp_methods[] = {
	ZEND_ME(Codecs_Cicp, __construct, arginfo_class_Codecs_Cicp___construct, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


static const zend_function_entry class_Codecs_DecodeOptions_methods[] = {
	ZEND_ME(Codecs_DecodeOptions, __construct, arginfo_class_Codecs_DecodeOptions___construct, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


static const zend_function_entry class_Codecs_AvifEncodeOptions_methods[] = {
	ZEND_ME(Codecs_AvifEncodeOptions, __construct, arginfo_class_Codecs_AvifEncodeOptions___construct, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


static const zend_function_entry class_Codecs_JxlEncodeOptions_methods[] = {
	ZEND_ME(Codecs_JxlEncodeOptions, __construct, arginfo_class_Codecs_JxlEncodeOptions___construct, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


static const zend_function_entry class_Codecs_Info_methods[] = {
	ZEND_ME(Codecs_Info, __construct, arginfo_class_Codecs_Info___construct, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


static const zend_function_entry class_Codecs_Image_methods[] = {
	ZEND_ME(Codecs_Image, fromPixels, arginfo_class_Codecs_Image_fromPixels, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Codecs_Image, fromPng, arginfo_class_Codecs_Image_fromPng, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Codecs_Image, fromGd, arginfo_class_Codecs_Image_fromGd, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Codecs_Image, width, arginfo_class_Codecs_Image_width, ZEND_ACC_PUBLIC)
	ZEND_ME(Codecs_Image, height, arginfo_class_Codecs_Image_height, ZEND_ACC_PUBLIC)
	ZEND_ME(Codecs_Image, bitDepth, arginfo_class_Codecs_Image_bitDepth, ZEND_ACC_PUBLIC)
	ZEND_ME(Codecs_Image, alphaMode, arginfo_class_Codecs_Image_alphaMode, ZEND_ACC_PUBLIC)
	ZEND_ME(Codecs_Image, provenance, arginfo_class_Codecs_Image_provenance, ZEND_ACC_PUBLIC)
	ZEND_ME(Codecs_Image, cicp, arginfo_class_Codecs_Image_cicp, ZEND_ACC_PUBLIC)
	ZEND_ME(Codecs_Image, iccProfile, arginfo_class_Codecs_Image_iccProfile, ZEND_ACC_PUBLIC)
	ZEND_ME(Codecs_Image, exif, arginfo_class_Codecs_Image_exif, ZEND_ACC_PUBLIC)
	ZEND_ME(Codecs_Image, xmp, arginfo_class_Codecs_Image_xmp, ZEND_ACC_PUBLIC)
	ZEND_ME(Codecs_Image, orientation, arginfo_class_Codecs_Image_orientation, ZEND_ACC_PUBLIC)
	ZEND_ME(Codecs_Image, memoryUsage, arginfo_class_Codecs_Image_memoryUsage, ZEND_ACC_PUBLIC)
	ZEND_ME(Codecs_Image, close, arginfo_class_Codecs_Image_close, ZEND_ACC_PUBLIC)
	ZEND_ME(Codecs_Image, toPng, arginfo_class_Codecs_Image_toPng, ZEND_ACC_PUBLIC)
	ZEND_ME(Codecs_Image, rgba16, arginfo_class_Codecs_Image_rgba16, ZEND_ACC_PUBLIC)
	ZEND_ME(Codecs_Image, rgba8, arginfo_class_Codecs_Image_rgba8, ZEND_ACC_PUBLIC)
	ZEND_ME(Codecs_Image, __clone, arginfo_class_Codecs_Image___clone, ZEND_ACC_PRIVATE)
	ZEND_ME(Codecs_Image, __debugInfo, arginfo_class_Codecs_Image___debugInfo, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


static const zend_function_entry class_Codecs_Codec_methods[] = {
	ZEND_ME(Codecs_Codec, decode, arginfo_class_Codecs_Codec_decode, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Codecs_Codec, decodeFile, arginfo_class_Codecs_Codec_decodeFile, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Codecs_Codec, decodeStream, arginfo_class_Codecs_Codec_decodeStream, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Codecs_Codec, encode, arginfo_class_Codecs_Codec_encode, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Codecs_Codec, encodeFile, arginfo_class_Codecs_Codec_encodeFile, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Codecs_Codec, encodeStream, arginfo_class_Codecs_Codec_encodeStream, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Codecs_Codec, transcode, arginfo_class_Codecs_Codec_transcode, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Codecs_Codec, probe, arginfo_class_Codecs_Codec_probe, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Codecs_Codec, probeFile, arginfo_class_Codecs_Codec_probeFile, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Codecs_Codec, probeStream, arginfo_class_Codecs_Codec_probeStream, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Codecs_Codec, supports, arginfo_class_Codecs_Codec_supports, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Codecs_Codec, versions, arginfo_class_Codecs_Codec_versions, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Codecs_Codec, version, arginfo_class_Codecs_Codec_version, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Codecs_Format(void)
{
	zend_class_entry *class_entry = zend_register_internal_enum("Codecs\\Format", IS_STRING, class_Codecs_Format_methods);

	zval enum_case_Avif_value;
	zend_string *enum_case_Avif_value_str = zend_string_init("avif", strlen("avif"), 1);
	ZVAL_STR(&enum_case_Avif_value, enum_case_Avif_value_str);
	zend_enum_add_case_cstr(class_entry, "Avif", &enum_case_Avif_value);

	zval enum_case_JpegXl_value;
	zend_string *enum_case_JpegXl_value_str = zend_string_init("jxl", strlen("jxl"), 1);
	ZVAL_STR(&enum_case_JpegXl_value, enum_case_JpegXl_value_str);
	zend_enum_add_case_cstr(class_entry, "JpegXl", &enum_case_JpegXl_value);

	zval enum_case_Heif_value;
	zend_string *enum_case_Heif_value_str = zend_string_init("heif", strlen("heif"), 1);
	ZVAL_STR(&enum_case_Heif_value, enum_case_Heif_value_str);
	zend_enum_add_case_cstr(class_entry, "Heif", &enum_case_Heif_value);

	return class_entry;
}

static zend_class_entry *register_class_Codecs_AlphaMode(void)
{
	zend_class_entry *class_entry = zend_register_internal_enum("Codecs\\AlphaMode", IS_STRING, class_Codecs_AlphaMode_methods);

	zval enum_case_Straight_value;
	zend_string *enum_case_Straight_value_str = zend_string_init("straight", strlen("straight"), 1);
	ZVAL_STR(&enum_case_Straight_value, enum_case_Straight_value_str);
	zend_enum_add_case_cstr(class_entry, "Straight", &enum_case_Straight_value);

	zval enum_case_Premultiplied_value;
	zend_string *enum_case_Premultiplied_value_str = zend_string_init("premultiplied", strlen("premultiplied"), 1);
	ZVAL_STR(&enum_case_Premultiplied_value, enum_case_Premultiplied_value_str);
	zend_enum_add_case_cstr(class_entry, "Premultiplied", &enum_case_Premultiplied_value);

	return class_entry;
}

static zend_class_entry *register_class_Codecs_ChromaSubsampling(void)
{
	zend_class_entry *class_entry = zend_register_internal_enum("Codecs\\ChromaSubsampling", IS_STRING, class_Codecs_ChromaSubsampling_methods);

	zval enum_case_Yuv444_value;
	zend_string *enum_case_Yuv444_value_str = zend_string_init("444", strlen("444"), 1);
	ZVAL_STR(&enum_case_Yuv444_value, enum_case_Yuv444_value_str);
	zend_enum_add_case_cstr(class_entry, "Yuv444", &enum_case_Yuv444_value);

	zval enum_case_Yuv422_value;
	zend_string *enum_case_Yuv422_value_str = zend_string_init("422", strlen("422"), 1);
	ZVAL_STR(&enum_case_Yuv422_value, enum_case_Yuv422_value_str);
	zend_enum_add_case_cstr(class_entry, "Yuv422", &enum_case_Yuv422_value);

	zval enum_case_Yuv420_value;
	zend_string *enum_case_Yuv420_value_str = zend_string_init("420", strlen("420"), 1);
	ZVAL_STR(&enum_case_Yuv420_value, enum_case_Yuv420_value_str);
	zend_enum_add_case_cstr(class_entry, "Yuv420", &enum_case_Yuv420_value);

	zval enum_case_Yuv400_value;
	zend_string *enum_case_Yuv400_value_str = zend_string_init("400", strlen("400"), 1);
	ZVAL_STR(&enum_case_Yuv400_value, enum_case_Yuv400_value_str);
	zend_enum_add_case_cstr(class_entry, "Yuv400", &enum_case_Yuv400_value);

	return class_entry;
}

static zend_class_entry *register_class_Codecs_MatrixCoefficients(void)
{
	zend_class_entry *class_entry = zend_register_internal_enum("Codecs\\MatrixCoefficients", IS_LONG, class_Codecs_MatrixCoefficients_methods);

	zval enum_case_Identity_value;
	ZVAL_LONG(&enum_case_Identity_value, 0);
	zend_enum_add_case_cstr(class_entry, "Identity", &enum_case_Identity_value);

	zval enum_case_Bt709_value;
	ZVAL_LONG(&enum_case_Bt709_value, 1);
	zend_enum_add_case_cstr(class_entry, "Bt709", &enum_case_Bt709_value);

	zval enum_case_Bt601_value;
	ZVAL_LONG(&enum_case_Bt601_value, 5);
	zend_enum_add_case_cstr(class_entry, "Bt601", &enum_case_Bt601_value);

	zval enum_case_Bt2020Ncl_value;
	ZVAL_LONG(&enum_case_Bt2020Ncl_value, 9);
	zend_enum_add_case_cstr(class_entry, "Bt2020Ncl", &enum_case_Bt2020Ncl_value);

	zval enum_case_YcgcoRe_value;
	ZVAL_LONG(&enum_case_YcgcoRe_value, 13);
	zend_enum_add_case_cstr(class_entry, "YcgcoRe", &enum_case_YcgcoRe_value);

	zval enum_case_YcgcoRo_value;
	ZVAL_LONG(&enum_case_YcgcoRo_value, 14);
	zend_enum_add_case_cstr(class_entry, "YcgcoRo", &enum_case_YcgcoRo_value);

	return class_entry;
}

static zend_class_entry *register_class_Codecs_Orientation(void)
{
	zend_class_entry *class_entry = zend_register_internal_enum("Codecs\\Orientation", IS_LONG, class_Codecs_Orientation_methods);

	zval enum_case_TopLeft_value;
	ZVAL_LONG(&enum_case_TopLeft_value, 1);
	zend_enum_add_case_cstr(class_entry, "TopLeft", &enum_case_TopLeft_value);

	zval enum_case_TopRight_value;
	ZVAL_LONG(&enum_case_TopRight_value, 2);
	zend_enum_add_case_cstr(class_entry, "TopRight", &enum_case_TopRight_value);

	zval enum_case_BottomRight_value;
	ZVAL_LONG(&enum_case_BottomRight_value, 3);
	zend_enum_add_case_cstr(class_entry, "BottomRight", &enum_case_BottomRight_value);

	zval enum_case_BottomLeft_value;
	ZVAL_LONG(&enum_case_BottomLeft_value, 4);
	zend_enum_add_case_cstr(class_entry, "BottomLeft", &enum_case_BottomLeft_value);

	zval enum_case_LeftTop_value;
	ZVAL_LONG(&enum_case_LeftTop_value, 5);
	zend_enum_add_case_cstr(class_entry, "LeftTop", &enum_case_LeftTop_value);

	zval enum_case_RightTop_value;
	ZVAL_LONG(&enum_case_RightTop_value, 6);
	zend_enum_add_case_cstr(class_entry, "RightTop", &enum_case_RightTop_value);

	zval enum_case_RightBottom_value;
	ZVAL_LONG(&enum_case_RightBottom_value, 7);
	zend_enum_add_case_cstr(class_entry, "RightBottom", &enum_case_RightBottom_value);

	zval enum_case_LeftBottom_value;
	ZVAL_LONG(&enum_case_LeftBottom_value, 8);
	zend_enum_add_case_cstr(class_entry, "LeftBottom", &enum_case_LeftBottom_value);

	return class_entry;
}

static zend_class_entry *register_class_Codecs_MetadataPolicy(void)
{
	zend_class_entry *class_entry = zend_register_internal_enum("Codecs\\MetadataPolicy", IS_STRING, class_Codecs_MetadataPolicy_methods);

	zval enum_case_Preserve_value;
	zend_string *enum_case_Preserve_value_str = zend_string_init("preserve", strlen("preserve"), 1);
	ZVAL_STR(&enum_case_Preserve_value, enum_case_Preserve_value_str);
	zend_enum_add_case_cstr(class_entry, "Preserve", &enum_case_Preserve_value);

	zval enum_case_Strip_value;
	zend_string *enum_case_Strip_value_str = zend_string_init("strip", strlen("strip"), 1);
	ZVAL_STR(&enum_case_Strip_value, enum_case_Strip_value_str);
	zend_enum_add_case_cstr(class_entry, "Strip", &enum_case_Strip_value);

	return class_entry;
}

static zend_class_entry *register_class_Codecs_ErrorCode(void)
{
	zend_class_entry *class_entry = zend_register_internal_enum("Codecs\\ErrorCode", IS_LONG, class_Codecs_ErrorCode_methods);

	zval enum_case_Ok_value;
	ZVAL_LONG(&enum_case_Ok_value, 0);
	zend_enum_add_case_cstr(class_entry, "Ok", &enum_case_Ok_value);

	zval enum_case_Container_value;
	ZVAL_LONG(&enum_case_Container_value, 1);
	zend_enum_add_case_cstr(class_entry, "Container", &enum_case_Container_value);

	zval enum_case_Bitstream_value;
	ZVAL_LONG(&enum_case_Bitstream_value, 2);
	zend_enum_add_case_cstr(class_entry, "Bitstream", &enum_case_Bitstream_value);

	zval enum_case_Metadata_value;
	ZVAL_LONG(&enum_case_Metadata_value, 3);
	zend_enum_add_case_cstr(class_entry, "Metadata", &enum_case_Metadata_value);

	zval enum_case_Limit_value;
	ZVAL_LONG(&enum_case_Limit_value, 4);
	zend_enum_add_case_cstr(class_entry, "Limit", &enum_case_Limit_value);

	zval enum_case_Dimension_value;
	ZVAL_LONG(&enum_case_Dimension_value, 5);
	zend_enum_add_case_cstr(class_entry, "Dimension", &enum_case_Dimension_value);

	zval enum_case_Pixels_value;
	ZVAL_LONG(&enum_case_Pixels_value, 6);
	zend_enum_add_case_cstr(class_entry, "Pixels", &enum_case_Pixels_value);

	zval enum_case_Unsupported_value;
	ZVAL_LONG(&enum_case_Unsupported_value, 7);
	zend_enum_add_case_cstr(class_entry, "Unsupported", &enum_case_Unsupported_value);

	zval enum_case_HeifNotCompiled_value;
	ZVAL_LONG(&enum_case_HeifNotCompiled_value, 8);
	zend_enum_add_case_cstr(class_entry, "HeifNotCompiled", &enum_case_HeifNotCompiled_value);

	zval enum_case_StreamIo_value;
	ZVAL_LONG(&enum_case_StreamIo_value, 9);
	zend_enum_add_case_cstr(class_entry, "StreamIo", &enum_case_StreamIo_value);

	zval enum_case_OutOfMemory_value;
	ZVAL_LONG(&enum_case_OutOfMemory_value, 10);
	zend_enum_add_case_cstr(class_entry, "OutOfMemory", &enum_case_OutOfMemory_value);

	zval enum_case_Internal_value;
	ZVAL_LONG(&enum_case_Internal_value, 11);
	zend_enum_add_case_cstr(class_entry, "Internal", &enum_case_Internal_value);

	return class_entry;
}

static zend_class_entry *register_class_Codecs_Exception(zend_class_entry *class_entry_RuntimeException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Codecs", "Exception", class_Codecs_Exception_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_RuntimeException);

	zval property_errorCode_default_value;
	ZVAL_UNDEF(&property_errorCode_default_value);
	zend_string *property_errorCode_name = zend_string_init("errorCode", sizeof("errorCode") - 1, 1);
	zend_string *property_errorCode_class_Codecs_ErrorCode = zend_string_init("Codecs\\ErrorCode", sizeof("Codecs\\ErrorCode")-1, 1);
	zend_declare_typed_property(class_entry, property_errorCode_name, &property_errorCode_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_CLASS(property_errorCode_class_Codecs_ErrorCode, 0, 0));
	zend_string_release(property_errorCode_name);

	return class_entry;
}

static zend_class_entry *register_class_Codecs_DecodeException(zend_class_entry *class_entry_Codecs_Exception)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Codecs", "DecodeException", class_Codecs_DecodeException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Codecs_Exception);

	return class_entry;
}

static zend_class_entry *register_class_Codecs_EncodeException(zend_class_entry *class_entry_Codecs_Exception)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Codecs", "EncodeException", class_Codecs_EncodeException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Codecs_Exception);

	return class_entry;
}

static zend_class_entry *register_class_Codecs_LimitException(zend_class_entry *class_entry_Codecs_Exception)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Codecs", "LimitException", class_Codecs_LimitException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Codecs_Exception);

	return class_entry;
}

static zend_class_entry *register_class_Codecs_UnsupportedFormatException(zend_class_entry *class_entry_Codecs_Exception)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Codecs", "UnsupportedFormatException", class_Codecs_UnsupportedFormatException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Codecs_Exception);

	return class_entry;
}

static zend_class_entry *register_class_Codecs_CorruptDataException(zend_class_entry *class_entry_Codecs_Exception)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Codecs", "CorruptDataException", class_Codecs_CorruptDataException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Codecs_Exception);

	return class_entry;
}

static zend_class_entry *register_class_Codecs_StreamException(zend_class_entry *class_entry_Codecs_Exception)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Codecs", "StreamException", class_Codecs_StreamException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Codecs_Exception);

	return class_entry;
}

static zend_class_entry *register_class_Codecs_EncodeOptions(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Codecs", "EncodeOptions", class_Codecs_EncodeOptions_methods);
	class_entry = zend_register_internal_interface(&ce);

	return class_entry;
}

static zend_class_entry *register_class_Codecs_Cicp(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Codecs", "Cicp", class_Codecs_Cicp_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_READONLY_CLASS;

	zval property_colourPrimaries_default_value;
	ZVAL_UNDEF(&property_colourPrimaries_default_value);
	zend_string *property_colourPrimaries_name = zend_string_init("colourPrimaries", sizeof("colourPrimaries") - 1, 1);
	zend_declare_typed_property(class_entry, property_colourPrimaries_name, &property_colourPrimaries_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(property_colourPrimaries_name);

	zval property_transferCharacteristics_default_value;
	ZVAL_UNDEF(&property_transferCharacteristics_default_value);
	zend_string *property_transferCharacteristics_name = zend_string_init("transferCharacteristics", sizeof("transferCharacteristics") - 1, 1);
	zend_declare_typed_property(class_entry, property_transferCharacteristics_name, &property_transferCharacteristics_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(property_transferCharacteristics_name);

	zval property_matrixCoefficients_default_value;
	ZVAL_UNDEF(&property_matrixCoefficients_default_value);
	zend_string *property_matrixCoefficients_name = zend_string_init("matrixCoefficients", sizeof("matrixCoefficients") - 1, 1);
	zend_declare_typed_property(class_entry, property_matrixCoefficients_name, &property_matrixCoefficients_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(property_matrixCoefficients_name);

	zval property_fullRange_default_value;
	ZVAL_UNDEF(&property_fullRange_default_value);
	zend_string *property_fullRange_name = zend_string_init("fullRange", sizeof("fullRange") - 1, 1);
	zend_declare_typed_property(class_entry, property_fullRange_name, &property_fullRange_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_BOOL));
	zend_string_release(property_fullRange_name);

	return class_entry;
}

static zend_class_entry *register_class_Codecs_DecodeOptions(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Codecs", "DecodeOptions", class_Codecs_DecodeOptions_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_READONLY_CLASS;

	zval property_maxDimension_default_value;
	ZVAL_UNDEF(&property_maxDimension_default_value);
	zend_string *property_maxDimension_name = zend_string_init("maxDimension", sizeof("maxDimension") - 1, 1);
	zend_declare_typed_property(class_entry, property_maxDimension_name, &property_maxDimension_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(property_maxDimension_name);

	zval property_maxPixels_default_value;
	ZVAL_UNDEF(&property_maxPixels_default_value);
	zend_string *property_maxPixels_name = zend_string_init("maxPixels", sizeof("maxPixels") - 1, 1);
	zend_declare_typed_property(class_entry, property_maxPixels_name, &property_maxPixels_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(property_maxPixels_name);

	zval property_ignoreIcc_default_value;
	ZVAL_UNDEF(&property_ignoreIcc_default_value);
	zend_string *property_ignoreIcc_name = zend_string_init("ignoreIcc", sizeof("ignoreIcc") - 1, 1);
	zend_declare_typed_property(class_entry, property_ignoreIcc_name, &property_ignoreIcc_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_BOOL));
	zend_string_release(property_ignoreIcc_name);

	zval property_ignoreExif_default_value;
	ZVAL_UNDEF(&property_ignoreExif_default_value);
	zend_string *property_ignoreExif_name = zend_string_init("ignoreExif", sizeof("ignoreExif") - 1, 1);
	zend_declare_typed_property(class_entry, property_ignoreExif_name, &property_ignoreExif_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_BOOL));
	zend_string_release(property_ignoreExif_name);

	zval property_ignoreXmp_default_value;
	ZVAL_UNDEF(&property_ignoreXmp_default_value);
	zend_string *property_ignoreXmp_name = zend_string_init("ignoreXmp", sizeof("ignoreXmp") - 1, 1);
	zend_declare_typed_property(class_entry, property_ignoreXmp_name, &property_ignoreXmp_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_BOOL));
	zend_string_release(property_ignoreXmp_name);

	zval property_keepOrientation_default_value;
	ZVAL_UNDEF(&property_keepOrientation_default_value);
	zend_string *property_keepOrientation_name = zend_string_init("keepOrientation", sizeof("keepOrientation") - 1, 1);
	zend_declare_typed_property(class_entry, property_keepOrientation_name, &property_keepOrientation_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_BOOL));
	zend_string_release(property_keepOrientation_name);

	zval property_firstFrameOnly_default_value;
	ZVAL_UNDEF(&property_firstFrameOnly_default_value);
	zend_string *property_firstFrameOnly_name = zend_string_init("firstFrameOnly", sizeof("firstFrameOnly") - 1, 1);
	zend_declare_typed_property(class_entry, property_firstFrameOnly_name, &property_firstFrameOnly_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_BOOL));
	zend_string_release(property_firstFrameOnly_name);

	zval property_matrixOverride_default_value;
	ZVAL_UNDEF(&property_matrixOverride_default_value);
	zend_string *property_matrixOverride_name = zend_string_init("matrixOverride", sizeof("matrixOverride") - 1, 1);
	zend_string *property_matrixOverride_class_Codecs_MatrixCoefficients = zend_string_init("Codecs\\MatrixCoefficients", sizeof("Codecs\\MatrixCoefficients")-1, 1);
	zend_declare_typed_property(class_entry, property_matrixOverride_name, &property_matrixOverride_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_CLASS(property_matrixOverride_class_Codecs_MatrixCoefficients, 0, MAY_BE_NULL));
	zend_string_release(property_matrixOverride_name);

	zval property_threads_default_value;
	ZVAL_UNDEF(&property_threads_default_value);
	zend_string *property_threads_name = zend_string_init("threads", sizeof("threads") - 1, 1);
	zend_declare_typed_property(class_entry, property_threads_name, &property_threads_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG|MAY_BE_NULL));
	zend_string_release(property_threads_name);

	return class_entry;
}

static zend_class_entry *register_class_Codecs_AvifEncodeOptions(zend_class_entry *class_entry_Codecs_EncodeOptions)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Codecs", "AvifEncodeOptions", class_Codecs_AvifEncodeOptions_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_READONLY_CLASS;
	zend_class_implements(class_entry, 1, class_entry_Codecs_EncodeOptions);

	zval property_quality_default_value;
	ZVAL_UNDEF(&property_quality_default_value);
	zend_string *property_quality_name = zend_string_init("quality", sizeof("quality") - 1, 1);
	zend_declare_typed_property(class_entry, property_quality_name, &property_quality_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(property_quality_name);

	zval property_qualityAlpha_default_value;
	ZVAL_UNDEF(&property_qualityAlpha_default_value);
	zend_string *property_qualityAlpha_name = zend_string_init("qualityAlpha", sizeof("qualityAlpha") - 1, 1);
	zend_declare_typed_property(class_entry, property_qualityAlpha_name, &property_qualityAlpha_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG|MAY_BE_NULL));
	zend_string_release(property_qualityAlpha_name);

	zval property_speed_default_value;
	ZVAL_UNDEF(&property_speed_default_value);
	zend_string *property_speed_name = zend_string_init("speed", sizeof("speed") - 1, 1);
	zend_declare_typed_property(class_entry, property_speed_name, &property_speed_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(property_speed_name);

	zval property_subsampling_default_value;
	ZVAL_UNDEF(&property_subsampling_default_value);
	zend_string *property_subsampling_name = zend_string_init("subsampling", sizeof("subsampling") - 1, 1);
	zend_string *property_subsampling_class_Codecs_ChromaSubsampling = zend_string_init("Codecs\\ChromaSubsampling", sizeof("Codecs\\ChromaSubsampling")-1, 1);
	zend_declare_typed_property(class_entry, property_subsampling_name, &property_subsampling_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_CLASS(property_subsampling_class_Codecs_ChromaSubsampling, 0, 0));
	zend_string_release(property_subsampling_name);

	zval property_matrix_default_value;
	ZVAL_UNDEF(&property_matrix_default_value);
	zend_string *property_matrix_name = zend_string_init("matrix", sizeof("matrix") - 1, 1);
	zend_string *property_matrix_class_Codecs_MatrixCoefficients = zend_string_init("Codecs\\MatrixCoefficients", sizeof("Codecs\\MatrixCoefficients")-1, 1);
	zend_declare_typed_property(class_entry, property_matrix_name, &property_matrix_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_CLASS(property_matrix_class_Codecs_MatrixCoefficients, 0, 0));
	zend_string_release(property_matrix_name);

	zval property_lossless_default_value;
	ZVAL_UNDEF(&property_lossless_default_value);
	zend_string *property_lossless_name = zend_string_init("lossless", sizeof("lossless") - 1, 1);
	zend_declare_typed_property(class_entry, property_lossless_name, &property_lossless_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_BOOL));
	zend_string_release(property_lossless_name);

	zval property_alpha_default_value;
	ZVAL_UNDEF(&property_alpha_default_value);
	zend_string *property_alpha_name = zend_string_init("alpha", sizeof("alpha") - 1, 1);
	zend_string *property_alpha_class_Codecs_AlphaMode = zend_string_init("Codecs\\AlphaMode", sizeof("Codecs\\AlphaMode")-1, 1);
	zend_declare_typed_property(class_entry, property_alpha_name, &property_alpha_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_CLASS(property_alpha_class_Codecs_AlphaMode, 0, 0));
	zend_string_release(property_alpha_name);

	zval property_metadata_default_value;
	ZVAL_UNDEF(&property_metadata_default_value);
	zend_string *property_metadata_name = zend_string_init("metadata", sizeof("metadata") - 1, 1);
	zend_string *property_metadata_class_Codecs_MetadataPolicy = zend_string_init("Codecs\\MetadataPolicy", sizeof("Codecs\\MetadataPolicy")-1, 1);
	zend_declare_typed_property(class_entry, property_metadata_name, &property_metadata_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_CLASS(property_metadata_class_Codecs_MetadataPolicy, 0, 0));
	zend_string_release(property_metadata_name);

	zval property_threads_default_value;
	ZVAL_UNDEF(&property_threads_default_value);
	zend_string *property_threads_name = zend_string_init("threads", sizeof("threads") - 1, 1);
	zend_declare_typed_property(class_entry, property_threads_name, &property_threads_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG|MAY_BE_NULL));
	zend_string_release(property_threads_name);

	return class_entry;
}

static zend_class_entry *register_class_Codecs_JxlEncodeOptions(zend_class_entry *class_entry_Codecs_EncodeOptions)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Codecs", "JxlEncodeOptions", class_Codecs_JxlEncodeOptions_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_READONLY_CLASS;
	zend_class_implements(class_entry, 1, class_entry_Codecs_EncodeOptions);

	zval property_quality_default_value;
	ZVAL_UNDEF(&property_quality_default_value);
	zend_string *property_quality_name = zend_string_init("quality", sizeof("quality") - 1, 1);
	zend_declare_typed_property(class_entry, property_quality_name, &property_quality_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(property_quality_name);

	zval property_effort_default_value;
	ZVAL_UNDEF(&property_effort_default_value);
	zend_string *property_effort_name = zend_string_init("effort", sizeof("effort") - 1, 1);
	zend_declare_typed_property(class_entry, property_effort_name, &property_effort_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(property_effort_name);

	zval property_decodingSpeed_default_value;
	ZVAL_UNDEF(&property_decodingSpeed_default_value);
	zend_string *property_decodingSpeed_name = zend_string_init("decodingSpeed", sizeof("decodingSpeed") - 1, 1);
	zend_declare_typed_property(class_entry, property_decodingSpeed_name, &property_decodingSpeed_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(property_decodingSpeed_name);

	zval property_lossless_default_value;
	ZVAL_UNDEF(&property_lossless_default_value);
	zend_string *property_lossless_name = zend_string_init("lossless", sizeof("lossless") - 1, 1);
	zend_declare_typed_property(class_entry, property_lossless_name, &property_lossless_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_BOOL));
	zend_string_release(property_lossless_name);

	zval property_alpha_default_value;
	ZVAL_UNDEF(&property_alpha_default_value);
	zend_string *property_alpha_name = zend_string_init("alpha", sizeof("alpha") - 1, 1);
	zend_string *property_alpha_class_Codecs_AlphaMode = zend_string_init("Codecs\\AlphaMode", sizeof("Codecs\\AlphaMode")-1, 1);
	zend_declare_typed_property(class_entry, property_alpha_name, &property_alpha_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_CLASS(property_alpha_class_Codecs_AlphaMode, 0, 0));
	zend_string_release(property_alpha_name);

	zval property_metadata_default_value;
	ZVAL_UNDEF(&property_metadata_default_value);
	zend_string *property_metadata_name = zend_string_init("metadata", sizeof("metadata") - 1, 1);
	zend_string *property_metadata_class_Codecs_MetadataPolicy = zend_string_init("Codecs\\MetadataPolicy", sizeof("Codecs\\MetadataPolicy")-1, 1);
	zend_declare_typed_property(class_entry, property_metadata_name, &property_metadata_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_CLASS(property_metadata_class_Codecs_MetadataPolicy, 0, 0));
	zend_string_release(property_metadata_name);

	zval property_threads_default_value;
	ZVAL_UNDEF(&property_threads_default_value);
	zend_string *property_threads_name = zend_string_init("threads", sizeof("threads") - 1, 1);
	zend_declare_typed_property(class_entry, property_threads_name, &property_threads_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG|MAY_BE_NULL));
	zend_string_release(property_threads_name);

	return class_entry;
}

static zend_class_entry *register_class_Codecs_Info(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Codecs", "Info", class_Codecs_Info_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_READONLY_CLASS;

	zval property_format_default_value;
	ZVAL_UNDEF(&property_format_default_value);
	zend_string *property_format_name = zend_string_init("format", sizeof("format") - 1, 1);
	zend_string *property_format_class_Codecs_Format = zend_string_init("Codecs\\Format", sizeof("Codecs\\Format")-1, 1);
	zend_declare_typed_property(class_entry, property_format_name, &property_format_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_CLASS(property_format_class_Codecs_Format, 0, 0));
	zend_string_release(property_format_name);

	zval property_width_default_value;
	ZVAL_UNDEF(&property_width_default_value);
	zend_string *property_width_name = zend_string_init("width", sizeof("width") - 1, 1);
	zend_declare_typed_property(class_entry, property_width_name, &property_width_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(property_width_name);

	zval property_height_default_value;
	ZVAL_UNDEF(&property_height_default_value);
	zend_string *property_height_name = zend_string_init("height", sizeof("height") - 1, 1);
	zend_declare_typed_property(class_entry, property_height_name, &property_height_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(property_height_name);

	zval property_bitDepth_default_value;
	ZVAL_UNDEF(&property_bitDepth_default_value);
	zend_string *property_bitDepth_name = zend_string_init("bitDepth", sizeof("bitDepth") - 1, 1);
	zend_declare_typed_property(class_entry, property_bitDepth_name, &property_bitDepth_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(property_bitDepth_name);

	zval property_alpha_default_value;
	ZVAL_UNDEF(&property_alpha_default_value);
	zend_string *property_alpha_name = zend_string_init("alpha", sizeof("alpha") - 1, 1);
	zend_declare_typed_property(class_entry, property_alpha_name, &property_alpha_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_BOOL));
	zend_string_release(property_alpha_name);

	zval property_cicp_default_value;
	ZVAL_UNDEF(&property_cicp_default_value);
	zend_string *property_cicp_name = zend_string_init("cicp", sizeof("cicp") - 1, 1);
	zend_string *property_cicp_class_Codecs_Cicp = zend_string_init("Codecs\\Cicp", sizeof("Codecs\\Cicp")-1, 1);
	zend_declare_typed_property(class_entry, property_cicp_name, &property_cicp_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_CLASS(property_cicp_class_Codecs_Cicp, 0, 0));
	zend_string_release(property_cicp_name);

	zval property_iccProfile_default_value;
	ZVAL_UNDEF(&property_iccProfile_default_value);
	zend_string *property_iccProfile_name = zend_string_init("iccProfile", sizeof("iccProfile") - 1, 1);
	zend_declare_typed_property(class_entry, property_iccProfile_name, &property_iccProfile_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	zend_string_release(property_iccProfile_name);

	zval property_orientation_default_value;
	ZVAL_UNDEF(&property_orientation_default_value);
	zend_string *property_orientation_name = zend_string_init("orientation", sizeof("orientation") - 1, 1);
	zend_string *property_orientation_class_Codecs_Orientation = zend_string_init("Codecs\\Orientation", sizeof("Codecs\\Orientation")-1, 1);
	zend_declare_typed_property(class_entry, property_orientation_name, &property_orientation_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_CLASS(property_orientation_class_Codecs_Orientation, 0, 0));
	zend_string_release(property_orientation_name);

	zval property_frameCount_default_value;
	ZVAL_UNDEF(&property_frameCount_default_value);
	zend_string *property_frameCount_name = zend_string_init("frameCount", sizeof("frameCount") - 1, 1);
	zend_declare_typed_property(class_entry, property_frameCount_name, &property_frameCount_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(property_frameCount_name);

	zval property_animated_default_value;
	ZVAL_UNDEF(&property_animated_default_value);
	zend_string *property_animated_name = zend_string_init("animated", sizeof("animated") - 1, 1);
	zend_declare_typed_property(class_entry, property_animated_name, &property_animated_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_BOOL));
	zend_string_release(property_animated_name);

	zval property_byteSize_default_value;
	ZVAL_UNDEF(&property_byteSize_default_value);
	zend_string *property_byteSize_name = zend_string_init("byteSize", sizeof("byteSize") - 1, 1);
	zend_declare_typed_property(class_entry, property_byteSize_name, &property_byteSize_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(property_byteSize_name);

	zval property_metadataBlocks_default_value;
	ZVAL_UNDEF(&property_metadataBlocks_default_value);
	zend_string *property_metadataBlocks_name = zend_string_init("metadataBlocks", sizeof("metadataBlocks") - 1, 1);
	zend_declare_typed_property(class_entry, property_metadataBlocks_name, &property_metadataBlocks_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_ARRAY));
	zend_string_release(property_metadataBlocks_name);

	return class_entry;
}

static zend_class_entry *register_class_Codecs_Image(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Codecs", "Image", class_Codecs_Image_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL;

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

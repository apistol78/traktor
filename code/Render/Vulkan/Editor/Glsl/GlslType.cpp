#include <sstream>
#include "Render/Vulkan/Editor/Glsl/GlslType.h"

namespace traktor
{
	namespace render
	{

std::wstring glsl_type_name(GlslType type)
{
	const wchar_t* c[] =
	{
		L"void",
		L"bool",
		L"float",
		L"vec2",
		L"vec3",
		L"vec4",
		L"mat4",
		L"tex",
		L"tex",
		L"tex"
	};
	T_ASSERT(type < sizeof_array(c));
	return c[type];
}

int32_t glsl_type_width(GlslType type)
{
	const int32_t w[] =
	{
		0,
		0,
		1,
		2,
		3,
		4,
		16,
		0,
		0,
		0
	};
	T_ASSERT(type < sizeof_array(w));
	return w[type];
}

GlslType glsl_from_data_type(DataType type)
{
	const GlslType c[] =
	{
		GtFloat,
		GtFloat2,
		GtFloat3,
		GtFloat4,
		GtFloat4,
		GtFloat4,
		GtFloat2,
		GtFloat4,
		GtFloat2,
		GtFloat4,
		GtFloat2,
		GtFloat4
	};
	T_ASSERT(type < sizeof_array(c));
	return c[type];
}

GlslType glsl_from_parameter_type(ParameterType type)
{
	const GlslType c[] =
	{
		GtFloat,
		GtFloat4,
		GtFloat4x4,
		GtTexture2D,
		GtTexture3D,
		GtTextureCube,
		GtStructBuffer
	};
	T_ASSERT(type < sizeof_array(c));
	return c[type];
}

std::wstring glslStorageType(DataType type)
{
	const wchar_t* c[] =
	{
		L"float",
		L"vec2",
		L"vec3",
		L"vec4",
		L"u8vec4",	// GL_EXT_shader_8bit_storage
		L"u8vec4",
		L"i16vec2",	// GL_EXT_shader_16bit_storage
		L"i16vec4",
		L"i16vec2",
		L"i16vec4",
		L"f16vec2",
		L"f16vec4"
	};
	T_ASSERT(type < sizeof_array(c));
	return c[type];
}

	}
}

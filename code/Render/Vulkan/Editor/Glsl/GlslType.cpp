#include <algorithm>
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
		L"int",
		L"ivec2",
		L"ivec3",
		L"ivec4",
		L"float",
		L"vec2",
		L"vec3",
		L"vec4",
		L"mat4",
		L"tex",
		L"tex",
		L"tex"
	};
	T_FATAL_ASSERT(type < sizeof_array(c));
	return c[type];
}

int32_t glsl_type_width(GlslType type)
{
	const int32_t w[] =
	{
		0,	// GtVoid
		0,	// GtBoolean
		1,	// GtInteger
		2,	// GtInteger2
		3,	// GtInteger3
		4,	// GtInteger4
		1,	// GtFloat
		2,	// GtFloat2
		3,	// GtFloat3
		4,	// GtFloat4
		16,	// GtFloat4x4
		0,	// GtTexture2D
		0,	// GtTexture3D
		0,	// GtTextureCube
		0	// GtStructBuffer
	};
	T_FATAL_ASSERT(type < sizeof_array(w));
	return w[type];
}

GlslType glsl_promote_to_float(GlslType type)
{
	const GlslType w[] =
	{
		GtVoid,		// GtVoid
		GtFloat,	// GtBoolean
		GtFloat,	// GtInteger
		GtFloat2,	// GtInteger2
		GtFloat3,	// GtInteger3
		GtFloat4,	// GtInteger4
		GtFloat,	// GtFloat
		GtFloat2,	// GtFloat2
		GtFloat3,	// GtFloat3
		GtFloat4,	// GtFloat4
		GtFloat4x4,	// GtFloat4x4
		GtVoid,		// GtTexture2D
		GtVoid,		// GtTexture3D
		GtVoid,		// GtTextureCube
		GtVoid		// GtStructBuffer
	};
	T_FATAL_ASSERT(type < sizeof_array(w));
	return w[type];
}

GlslType glsl_degrade_to_integer(GlslType type)
{
	const GlslType w[] =
	{
		GtVoid,		// GtVoid
		GtInteger,	// GtBoolean
		GtInteger,	// GtInteger
		GtInteger2,	// GtInteger2
		GtInteger3,	// GtInteger3
		GtInteger4,	// GtInteger4
		GtInteger,	// GtFloat
		GtInteger2,	// GtFloat2
		GtInteger3,	// GtFloat3
		GtInteger4,	// GtFloat4
		GtVoid,		// GtFloat4x4
		GtVoid,		// GtTexture2D
		GtVoid,		// GtTexture3D
		GtVoid,		// GtTextureCube
		GtVoid		// GtStructBuffer
	};
	T_FATAL_ASSERT(type < sizeof_array(w));
	return w[type];	
}

GlslType glsl_precedence(GlslType typeA, GlslType typeB)
{
	bool intA = (typeA >= GtBoolean && typeA <= GtInteger4);
	bool intB = (typeB >= GtBoolean && typeB <= GtInteger4);

	if (intA && !intB)
		typeA = glsl_promote_to_float(typeA);
	if (!intA && intB)
		typeB = glsl_promote_to_float(typeB);

	return std::max(typeA, typeB);
}

GlslType glsl_from_data_type(DataType type)
{
	const GlslType c[] =
	{
		GtFloat,	// DtFloat1
		GtFloat2,	// DtFloat2
		GtFloat3,	// DtFloat3
		GtFloat4,	// DtFloat4
		GtInteger4,	// DtByte4
		GtFloat4,	// DtByte4N
		GtInteger2,	// DtShort2
		GtInteger4,	// DtShort4
		GtFloat2,	// DtShort2N
		GtFloat4,	// DtShort4N
		GtFloat2,	// DtHalf2
		GtFloat4,	// DtHalf4,
		GtInteger,	// DtInteger1
		GtInteger2,	// DtInteger2
		GtInteger3,	// DtInteger3
		GtInteger4	// DtInteger4
	};
	T_FATAL_ASSERT(type < sizeof_array(c));
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
	T_FATAL_ASSERT(type < sizeof_array(c));
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
		L"f16vec4",
		L"int",
		L"ivec2",
		L"ivec3",
		L"ivec4"
	};
	T_FATAL_ASSERT(type < sizeof_array(c));
	return c[type];
}

	}
}

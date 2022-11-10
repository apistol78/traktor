/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/String.h"
#include "Render/Editor/Glsl/GlslType.h"

namespace traktor::render
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
		L"tex",			// GtTexture2D
		L"tex",
		L"tex",			// GtTextureCube
		L"",			// GtStructBuffer
		L"image2D",		// GtImage2D
		L"image3D",
		L"imageCube"	// GtImageCube
	};
	T_FATAL_ASSERT((int32_t)type < sizeof_array(c));
	return c[(int32_t)type];
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
		0,	// GtStructBuffer
		0,	// GtImage2D
		0,	// GtImage3D
		0	// GtImageCube
	};
	T_FATAL_ASSERT((int32_t)type < sizeof_array(w));
	return w[(int32_t)type];
}

GlslType glsl_promote_to_float(GlslType type)
{
	const GlslType w[] =
	{
		GlslType::Void,		// Void
		GlslType::Float,	// Boolean
		GlslType::Float,	// Integer
		GlslType::Float2,	// Integer2
		GlslType::Float3,	// Integer3
		GlslType::Float4,	// Integer4
		GlslType::Float,	// Float
		GlslType::Float2,	// Float2
		GlslType::Float3,	// Float3
		GlslType::Float4,	// Float4
		GlslType::Float4x4,	// Float4x4
		GlslType::Void,		// Texture2D
		GlslType::Void,		// Texture3D
		GlslType::Void,		// TextureCube
		GlslType::Void,		// StructBuffer
		GlslType::Void,		// Image2D
		GlslType::Void,		// Image3D
		GlslType::Void		// ImageCube
	};
	T_FATAL_ASSERT((int32_t)type < sizeof_array(w));
	return w[(int32_t)type];
}

GlslType glsl_degrade_to_integer(GlslType type)
{
	const GlslType w[] =
	{
		GlslType::Void,		// Void
		GlslType::Integer,	// Boolean
		GlslType::Integer,	// Integer
		GlslType::Integer2,	// Integer2
		GlslType::Integer3,	// Integer3
		GlslType::Integer4,	// Integer4
		GlslType::Integer,	// Float
		GlslType::Integer2,	// Float2
		GlslType::Integer3,	// Float3
		GlslType::Integer4,	// Float4
		GlslType::Void,		// Float4x4
		GlslType::Void,		// Texture2D
		GlslType::Void,		// Texture3D
		GlslType::Void,		// TextureCube
		GlslType::Void,		// StructBuffer
		GlslType::Void,		// Image2D
		GlslType::Void,		// Image3D
		GlslType::Void		// ImageCube
	};
	T_FATAL_ASSERT((int32_t)type < sizeof_array(w));
	return w[(int32_t)type];	
}

GlslType glsl_precedence(GlslType typeA, GlslType typeB)
{
	bool intA = (typeA >= GlslType::Boolean && typeA <= GlslType::Integer4);
	bool intB = (typeB >= GlslType::Boolean && typeB <= GlslType::Integer4);

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
		GlslType::Float,	// DtFloat1
		GlslType::Float2,	// DtFloat2
		GlslType::Float3,	// DtFloat3
		GlslType::Float4,	// DtFloat4
		GlslType::Integer4,	// DtByte4
		GlslType::Float4,	// DtByte4N
		GlslType::Integer2,	// DtShort2
		GlslType::Integer4,	// DtShort4
		GlslType::Float2,	// DtShort2N
		GlslType::Float4,	// DtShort4N
		GlslType::Float2,	// DtHalf2
		GlslType::Float4,	// DtHalf4,
		GlslType::Integer,	// DtInteger1
		GlslType::Integer2,	// DtInteger2
		GlslType::Integer3,	// DtInteger3
		GlslType::Integer4	// DtInteger4
	};
	return c[type];
}

GlslType glsl_from_parameter_type(ParameterType type)
{
	const GlslType c[] =
	{
		GlslType::Float,
		GlslType::Float4,
		GlslType::Float4x4,
		GlslType::Texture2D,
		GlslType::Texture3D,
		GlslType::TextureCube,
		GlslType::StructBuffer,
		GlslType::Image2D,
		GlslType::Image3D,
		GlslType::ImageCube
	};
	T_FATAL_ASSERT((int32_t)type < sizeof_array(c));
	return c[(int32_t)type];
}

std::wstring glsl_storage_type(DataType type)
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
	return c[type];
}

std::wstring glsl_vertex_attribute_name(DataUsage usage, int32_t index)
{
	const wchar_t* s[] =
	{
		L"Position",
		L"Normal",
		L"Tangent",
		L"Binormal",
		L"Color",
		L"Custom",
		L""
	};
	return str(L"in_%ls_%d", s[(int)usage], index);
}

int32_t glsl_vertex_attribute_location(DataUsage usage, int32_t index)
{
	const int32_t base[] =
	{
		0,
		1,
		2,
		3,
		4,
		5,
		16
	};
	int32_t location = base[(int)usage] + index;
	return (location < base[(int)usage + 1]) ? location : -1;
}

}

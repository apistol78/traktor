/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <sstream>
#include "Render/OpenGL/Glsl/GlslType.h"

namespace traktor
{
	namespace render
	{

std::wstring glsl_vertex_attr_name(DataUsage usage, int index)
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
	std::wstringstream ss;
	ss << L"in_" << s[usage] << index;
	return ss.str();
}

int32_t glsl_vertex_attr_location(DataUsage usage, int index)
{
	int32_t base[] =
	{
		0,
		1,
		2,
		3,
		4,
		5,
		16
	};
	int32_t location = base[int(usage)] + index;
	return (location < base[int(usage) + 1]) ? location : -1;
}

std::wstring glsl_type_name(GlslType type)
{
	const wchar_t* c[] =
	{
		L"void",
		L"float",
		L"vec2",
		L"vec3",
		L"vec4",
		L"mat4",
		L"tex",
		L"tex",
		L"tex"
	};
	T_ASSERT (type < sizeof_array(c));
	return c[type];
}

int32_t glsl_type_width(GlslType type)
{
	const int32_t w[] =
	{
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
	T_ASSERT (type < sizeof_array(w));
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
	T_ASSERT (type < sizeof_array(c));
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
		GtTextureCube
	};
	T_ASSERT (type < sizeof_array(c));
	return c[type];
}

	}
}

#include <sstream>
#include "Render/OpenGL/GlslType.h"

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
		L"Custom"
	};
	std::wstringstream ss;
	ss << L"in_" << s[usage] << index;
	return ss.str();
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
		L"mat4"
	};
	return c[type];
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
	return c[type];
}

	}
}

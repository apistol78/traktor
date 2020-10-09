#include "Core/Misc/String.h"
#include "Render/Vulkan/Editor/Glsl/GlslVariable.h"

namespace traktor
{
	namespace render
	{

GlslVariable::GlslVariable(const Node* node, const std::wstring& name, GlslType type)
:	m_node(node)
,	m_name(name)
,	m_type(type)
{
}

std::wstring GlslVariable::cast(GlslType to) const
{
	const wchar_t* c[8][8] =
	{
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, L"%", L"vec2(%, %)", L"vec3(%, %, %)", L"vec4(%, %, %, %)", 0, 0 },
		{ 0, 0, L"%.x", L"%", L"vec3(%.xy, 0.0)", L"vec4(%.xy, 0.0, 0.0)", 0, 0 },
		{ 0, 0, L"%.x", L"%.xy", L"%", L"vec4(%.xyz, 0.0)", 0, 0 },
		{ 0, 0, L"%.x", L"%.xy", L"%.xyz", L"%", 0, 0 },
		{ 0, 0, 0, 0, 0, 0, L"%", 0 },
		{ 0, 0, 0, 0, 0, 0, 0, L"%" }
	};

	const wchar_t* f = c[m_type][to];
	return f ? replaceAll(f, L"%", m_name) : m_name;
}

std::wstring GlslVariable::castToInteger(GlslType to) const
{
	const wchar_t* c[8][8] =
	{
		{ 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, L"%", L"ivec2(%, %)", L"ivec3(%, %, %)", L"ivec4(%, %, %, %)", 0, 0 },
		{ 0, 0, L"%.x", L"ivec2(%)", L"ivec3(%.xy, 0)", L"ivec4(%.xy, 0, 0)", 0, 0 },
		{ 0, 0, L"%.x", L"ivec2(%.xy)", L"ivec3(%)", L"ivec4(%.xyz, 0)", 0, 0 },
		{ 0, 0, L"%.x", L"ivec2(%.xy)", L"ivec3(%.xyz)", L"ivec4(%)", 0, 0 },
		{ 0, 0, 0, 0, 0, 0, L"%", 0 },
		{ 0, 0, 0, 0, 0, 0, 0, L"%" }
	};

	const wchar_t* f = c[m_type][to];
	return f ? replaceAll(f, L"%", m_name) : m_name;
}

GlslVariable& GlslVariable::operator = (const GlslVariable& other)
{
	m_node = other.m_node;
	m_name = other.m_name;
	m_type = other.m_type;
	return *this;
}

	}
}

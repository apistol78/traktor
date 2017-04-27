/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/String.h"
#include "Render/Vulkan/Glsl/GlslVariable.h"

namespace traktor
{
	namespace render
	{

GlslVariable::GlslVariable()
:	m_type(GtVoid)
{
}

GlslVariable::GlslVariable(const std::wstring& name, GlslType type)
:	m_name(name)
,	m_type(type)
{
}

std::wstring GlslVariable::cast(GlslType to) const
{
	const wchar_t* c[7][7] =
	{
		{ 0, 0, 0, 0, 0, 0, 0 },
		{ 0, L"%", L"vec2(%, %)", L"vec3(%, %, %)", L"vec4(%, %, %, %)", 0, 0 },
		{ 0, L"%.x", L"%", L"vec3(%.xy, 0.0)", L"vec4(%.xy, 0.0, 0.0)", 0, 0 },
		{ 0, L"%.x", L"%.xy", L"%", L"vec4(%.xyz, 0.0)", 0, 0 },
		{ 0, L"%.x", L"%.xy", L"%.xyz", L"%", 0, 0 },
		{ 0, 0, 0, 0, 0, L"%", 0 },
		{ 0, 0, 0, 0, 0, 0, L"%" }
	};
	
	const wchar_t* f = c[m_type][to];
	return f ? replaceAll< std::wstring >(f, L"%", m_name) : m_name;
}

	}
}

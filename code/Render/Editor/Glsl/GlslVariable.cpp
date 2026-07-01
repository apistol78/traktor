/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Glsl/GlslVariable.h"

#include "Core/Misc/String.h"

namespace traktor::render
{
namespace
{

const wchar_t* c_castFormat[10][10] = {
	//       |      I      |         I2        |       I3         |          I4         |    F         |       F1     |         F2         |           F3              |
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	/*  I */ { L"%", L"ivec2(%, %)", L"ivec3(%, %, %)", L"ivec4(%, %, %, %)", L"float(%)", L"vec2(%, %)", L"vec3(%, %, %)", L"vec4(%, %, %, %)" },
	/* I2 */ { L"%.x", L"%", L"ivec3(%.xy, 0)", L"ivec4(%.xy, 0, 0)", L"float(%.x)", L"vec2(%)", L"vec3(%.xy, 0.0)", L"vec4(%.xy, 0.0, 0.0)" },
	/* I3 */ { L"%.x", L"%.xy", L"%", L"ivec4(%.xyz, 0)", L"float(%.x)", L"vec2(%.xy)", L"vec3(%)", L"vec4(%.xyz, 0.0)" },
	/* I4 */ { L"%.x", L"%.xy", L"%.xyz", L"%", L"float(%.x)", L"vec2(%.xy)", L"vec3(%.xyz)", L"vec4(%)" },
	/*  F */ { L"int(%)", L"ivec2(%, %)", L"ivec3(%, %, %)", L"ivec4(%, %, %, %)", L"%", L"vec2(%, %)", L"vec3(%, %, %)", L"vec4(%, %, %, %)" },
	/* F2 */ { L"int(%.x)", L"ivec2(%)", L"ivec3(%.xy, 0)", L"ivec4(%.xy, 0, 0)", L"%.x", L"%", L"vec3(%.xy, 0.0)", L"vec4(%.xy, 0.0, 0.0)" },
	/* F3 */ { L"int(%.x)", L"ivec2(%.xy)", L"ivec3(%)", L"ivec4(%.xyz, 0)", L"%.x", L"%.xy", L"%", L"vec4(%.xyz, 0.0)" },
	/* F4 */ { L"int(%.x)", L"ivec2(%.xy)", L"ivec3(%.xyz)", L"ivec4(%)", L"%.x", L"%.xy", L"%.xyz", L"%" },
};

}

GlslVariable::GlslVariable(const Node* node, const std::wstring& name, GlslType type, bool array)
	: m_node(node)
	, m_name(name)
	, m_type(type)
	, m_array(array)
{
}

GlslVariable::GlslVariable(const Node* node, const std::wstring& name, const std::wstring& structTypeName, const StructDeclaration& structDeclaration, GlslType type)
	: m_node(node)
	, m_name(name)
	, m_structTypeName(structTypeName)
	, m_structDeclaration(structDeclaration)
	, m_type(type)
{
}

std::wstring GlslVariable::cast(GlslType to) const
{
	if (m_type == GlslType::Void || m_type == GlslType::Boolean || m_type >= GlslType::Float4x4 || to >= GlslType::Float4x4)
		return m_name;

	const int32_t base = (int32_t)GlslType::Integer;
	const wchar_t* f = c_castFormat[(int32_t)m_type - base][(int32_t)to - base];
	return f ? replaceAll(f, L"%", m_name) : m_name;
}

GlslVariable& GlslVariable::operator=(const GlslVariable& other)
{
	m_node = other.m_node;
	m_name = other.m_name;
	m_structTypeName = other.m_structTypeName;
	m_structDeclaration = other.m_structDeclaration;
	m_type = other.m_type;
	m_array = other.m_array;
	return *this;
}

}

/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/String.h"
#include "Render/Dx11/Editor/Hlsl/HlslVariable.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const wchar_t* c_castFormat[10][10] =
{
	//       |      I      |         I2        |       I3         |          I4         |    F         |       F1       |         F2          |           F3              |
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	/*  I */ {         L"%",      L"int2(%, %)",  L"int3(%, %, %)",  L"int4(%, %, %, %)",   L"float(%)", L"float2(%, %)",   L"float3(%, %, %)",      L"float4(%, %, %, %)" },
	/* I2 */ {       L"%.x",               L"%",  L"int3(%.xy, 0)",  L"int4(%.xy, 0, 0)", L"float(%.x)",    L"float2(%)", L"float3(%.xy, 0.0)",  L"float4(%.xy, 0.0, 0.0)" },
	/* I3 */ {       L"%.x",            L"%.xy",              L"%",    L"int4(%.xyz, 0)", L"float(%.x)", L"float2(%.xy)",         L"float3(%)",      L"float4(%.xyz, 0.0)" },
	/* I4 */ {       L"%.x",            L"%.xy",          L"%.xyz",                 L"%", L"float(%.x)", L"float2(%.xy)",     L"float3(%.xyz)",               L"float4(%)" },
	/*  F */ {    L"int(%)",      L"int2(%, %)",  L"int3(%, %, %)",  L"int4(%, %, %, %)",          L"%", L"float2(%, %)",   L"float3(%, %, %)",      L"float4(%, %, %, %)" },
	/* F2 */ {  L"int(%.x)",         L"int2(%)",  L"int3(%.xy, 0)",  L"int4(%.xy, 0, 0)",        L"%.x",            L"%", L"float3(%.xy, 0.0)",  L"float4(%.xy, 0.0, 0.0)" },
	/* F3 */ {  L"int(%.x)",      L"int2(%.xy)",        L"int3(%)",    L"int4(%.xyz, 0)",        L"%.x",         L"%.xy",                 L"%",      L"float4(%.xyz, 0.0)" },
	/* F4 */ {  L"int(%.x)",      L"int2(%.xy)",    L"int3(%.xyz)",           L"int4(%)",        L"%.x",         L"%.xy",             L"%.xyz",                       L"%" },
};

		}

HlslVariable::HlslVariable(const Node* node, const std::wstring& name, HlslType type)
:	m_node(node)
,	m_name(name)
,	m_type(type)
{
}

std::wstring HlslVariable::cast(HlslType to) const
{
	if (m_type == HtVoid || m_type == HtBoolean || m_type >= HtFloat4x4 || to >= HtFloat4x4)
		return m_name;

	const wchar_t* f = c_castFormat[m_type - HtInteger][to - HtInteger];
	return f ? replaceAll(f, L"%", m_name) : m_name;
}

HlslVariable& HlslVariable::operator = (const HlslVariable& other)
{
	m_node = other.m_node;
	m_name = other.m_name;
	m_type = other.m_type;
	return *this;
}

	}
}

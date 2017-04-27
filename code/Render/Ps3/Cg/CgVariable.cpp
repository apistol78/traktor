/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/String.h"
#include "Render/Ps3/Cg/CgVariable.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.CgVariable", CgVariable, Object)

CgVariable::CgVariable()
:	m_type(CtVoid)
{
}

CgVariable::CgVariable(const std::wstring& name, CgType type)
:	m_name(name)
,	m_type(type)
{
}

std::wstring CgVariable::cast(CgType to) const
{
	const wchar_t* c[7][7] =
	{
		{ 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, L"%", L"%.xx", L"%.xxx", L"%.xxxx", 0 },
		{ 0, 0, L"%.x", L"%", L"float3(%.xy, 0)", L"float4(%.xy, 0, 0)", 0 },
		{ 0, 0, L"%.x", L"%.xy", L"%", L"float4(%.xyz, 0)", 0 },
		{ 0, 0, L"%.x", L"%.xy", L"%.xyz", L"%", 0 },
		{ 0, 0, 0, 0, 0, 0, L"%" }
	};
	
	const wchar_t* f = c[m_type][to];
	return f ? replaceAll< std::wstring >(f, L"%", m_name) : m_name;
}

	}
}

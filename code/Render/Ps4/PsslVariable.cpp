#include "Core/Misc/String.h"
#include "Render/Ps4/PsslVariable.h"

namespace traktor
{
	namespace render
	{

PsslVariable::PsslVariable()
:	m_type(PsslVoid)
{
}

PsslVariable::PsslVariable(const std::wstring& name, PsslType type)
:	m_name(name)
,	m_type(type)
{
}

std::wstring PsslVariable::cast(PsslType to) const
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

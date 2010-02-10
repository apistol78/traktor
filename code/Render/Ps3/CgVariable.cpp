#include "Render/Ps3/CgVariable.h"
#include "Core/Misc/String.h"

namespace traktor
{
	namespace render
	{

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
	T_ASSERT (f);

	return replaceAll< std::wstring >(f, L"%", m_name);
}

	}
}

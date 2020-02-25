#include "Core/Misc/String.h"
#include "Render/Dx11/Editor/Hlsl/HlslVariable.h"

namespace traktor
{
	namespace render
	{

HlslVariable::HlslVariable()
:	m_node(nullptr)
,	m_type(HtVoid)
{
}

HlslVariable::HlslVariable(const Node* node, const std::wstring& name, HlslType type)
:	m_node(node)
,	m_name(name)
,	m_type(type)
{
}

std::wstring HlslVariable::cast(HlslType to) const
{
	const wchar_t* c[8][8] =
	{
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, L"%", L"%.xx", L"%.xxx", L"%.xxxx", 0, 0 },
		{ 0, 0, L"%.x", L"%", L"float3(%.xy, 0)", L"float4(%.xy, 0, 0)", 0, 0 },
		{ 0, 0, L"%.x", L"%.xy", L"%", L"float4(%.xyz, 0)", 0, 0 },
		{ 0, 0, L"%.x", L"%.xy", L"%.xyz", L"%", 0, 0 },
		{ 0, 0, 0, 0, 0, 0, L"%", 0 },
		{ 0, 0, 0, 0, 0, 0, 0, L"%" },
	};

	const wchar_t* f = c[m_type][to];
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

#include <cassert>
#include "Render/Ps3/CgVariable.h"
#include "Core/Misc/StringUtils.h"

namespace traktor
{
	namespace render
	{

CgVariable::CgVariable(const std::wstring& name, CgType type)
:	m_name(name)
,	m_type(type)
{
}

const std::wstring& CgVariable::getName() const
{
	return m_name;
}

CgType CgVariable::getType() const
{
	return m_type;
}

std::wstring CgVariable::cast(CgType to) const
{
	const char* c[5][5] =
	{
		{ 0, 0, 0, 0, 0 },
		{ 0, "%", "%.xx", "%.xxx", "%.xxxx" },
		{ 0, 0, "%", "float3(%.xy, 0)", "float4(%.xy, 0, 0)" },
		{ 0, 0, 0, "%", "float4(%.xyz, 0)" },
		{ 0, 0, 0, "%.xyz", "%" }
	};
	
	const char* f = c[m_type][to];
	assert (f);

	return replaceAll(f, "%", m_name);
}

	}
}

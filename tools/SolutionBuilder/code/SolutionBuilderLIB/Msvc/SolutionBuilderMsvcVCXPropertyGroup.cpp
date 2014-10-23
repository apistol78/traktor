#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/MemberStl.h>
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcVCXPropertyGroup.h"

using namespace traktor;

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"SolutionBuilderMsvcVCXPropertyGroup", 0, SolutionBuilderMsvcVCXPropertyGroup, ISerializable)

bool SolutionBuilderMsvcVCXPropertyGroup::generate(
	GeneratorContext& context,
	const Solution* solution,
	const Project* project,
	OutputStream& os
) const
{
	os << L"<PropertyGroup Label=\"" << m_label << L"\">" << Endl;
	for (std::map< std::wstring, std::wstring >::const_iterator i = m_values.begin(); i != m_values.end(); ++i)
		os << L"<" << i->first << L">" << i->second << L"</" << i->first << L">" << Endl;
	os << L"</PropertyGroup>" << Endl;
	return true;
}

void SolutionBuilderMsvcVCXPropertyGroup::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"label", m_label);
	s >> MemberStlMap< std::wstring, std::wstring >(L"values", m_values);
}

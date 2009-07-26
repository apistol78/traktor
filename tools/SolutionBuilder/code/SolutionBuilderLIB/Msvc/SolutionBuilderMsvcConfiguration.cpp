#include <Core/Serialization/Serializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Serialization/MemberStl.h>
#include <Core/Serialization/MemberStaticArray.h>
#include <Core/Serialization/MemberRef.h>
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcConfiguration.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcTool.h"
#include "SolutionBuilderLIB/Msvc/GeneratorContext.h"
#include "SolutionBuilderLIB/Configuration.h"

using namespace traktor;

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"SolutionBuilderMsvcConfiguration", SolutionBuilderMsvcConfiguration, Serializable)

bool SolutionBuilderMsvcConfiguration::generate(
	GeneratorContext& context,
	Solution* solution,
	Project* project,
	Configuration* configuration,
	const std::wstring& platform,
	traktor::OutputStream& os
) const
{
	os << L"<Configuration" << Endl;
	os << IncreaseIndent;

	for (std::map< std::wstring, std::wstring >::const_iterator i = m_staticOptions.begin(); i != m_staticOptions.end(); ++i)
		os << i->first << L"=\"" << context.format(i->second) << L"\"" << Endl;

	os << L">" << Endl;

	const RefArray< SolutionBuilderMsvcTool >& tools = m_tools[int(configuration->getTargetProfile())];
	for (RefArray< SolutionBuilderMsvcTool >::const_iterator i = tools.begin(); i != tools.end(); ++i)
	{
		if (!(*i)->generate(context, solution, project, configuration, os))
			return false;
	}

	os << DecreaseIndent;
	os << L"</Configuration>" << Endl;

	return true;
}

bool SolutionBuilderMsvcConfiguration::serialize(Serializer& s)
{
	s >> MemberStlMap< std::wstring, std::wstring >(L"staticOptions", m_staticOptions);
	s >> MemberStaticArray< RefArray< SolutionBuilderMsvcTool >, 2, MemberRefArray< SolutionBuilderMsvcTool > >(L"tools", m_tools);
	return true;
}

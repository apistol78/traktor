#include "Core/Io/OutputStream.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Core/Serialization/MemberRefArray.h"
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcConfiguration.h"
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcTool.h"
#include "SolutionBuilder/Msvc/GeneratorContext.h"
#include "SolutionBuilder/Configuration.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"SolutionBuilderMsvcConfiguration", 0, SolutionBuilderMsvcConfiguration, ISerializable)

bool SolutionBuilderMsvcConfiguration::generate(
	GeneratorContext& context,
	Solution* solution,
	Project* project,
	Configuration* configuration,
	const std::wstring& platform,
	OutputStream& os
) const
{
	os << L"<Configuration" << Endl;
	os << IncreaseIndent;

	for (const auto& it : m_staticOptions)
		os << it.first << L"=\"" << context.format(it.second) << L"\"" << Endl;

	os << L">" << Endl;

	const auto& tools = m_tools[int(configuration->getTargetProfile())];
	for (auto tool : tools)
	{
		if (!tool->generate(context, solution, project, configuration, os))
			return false;
	}

	os << DecreaseIndent;
	os << L"</Configuration>" << Endl;
	return true;
}

void SolutionBuilderMsvcConfiguration::serialize(ISerializer& s)
{
	s >> MemberStlMap< std::wstring, std::wstring >(L"staticOptions", m_staticOptions);
	s >> MemberStaticArray< RefArray< SolutionBuilderMsvcTool >, 2, MemberRefArray< SolutionBuilderMsvcTool > >(L"tools", m_tools);
}

	}
}

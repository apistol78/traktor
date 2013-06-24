#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Serialization/MemberStl.h>
#include "SolutionBuilderLIB/Configuration.h"
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/Msvc/GeneratorContext.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcCompilerTool.h"

using namespace traktor;

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"SolutionBuilderMsvcCompilerTool", 0, SolutionBuilderMsvcCompilerTool, SolutionBuilderMsvcTool)

SolutionBuilderMsvcCompilerTool::SolutionBuilderMsvcCompilerTool()
:	m_resolvePaths(false)
{
}

bool SolutionBuilderMsvcCompilerTool::generate(GeneratorContext& context, Solution* solution, Project* project, Configuration* configuration, OutputStream& os) const
{
	os << L"<Tool" << Endl;
	os << IncreaseIndent;
	os << L"Name=\"VCCLCompilerTool\"" << Endl;

	// Additional options.
	os << L"AdditionalOptions=\"";
	std::map< std::wstring, std::wstring >::const_iterator i0 = m_staticOptions.find(L"AdditionalOptions");
	if (i0 != m_staticOptions.end())
	{
		os << i0->second;
		if (!configuration->getAdditionalCompilerOptions().empty())
			os << L" " << configuration->getAdditionalCompilerOptions();
	}
	else
		os << configuration->getAdditionalCompilerOptions();

	os << L"\"" << Endl;

	// Include directories.
	os << L"AdditionalIncludeDirectories=\"";
	for (std::vector< std::wstring >::const_iterator i = configuration->getIncludePaths().begin(); i != configuration->getIncludePaths().end(); ++i)
	{
		std::wstring includePath = context.getProjectRelativePath(*i, m_resolvePaths);
		os << includePath << L";";
	}

	std::map< std::wstring, std::wstring >::const_iterator i1 = m_staticOptions.find(L"AdditionalIncludeDirectories");
	if (i1 != m_staticOptions.end())
		os << context.format(i1->second) << L";";

	os << L"\"" << Endl;

	// Preprocessor definitions.
	os << L"PreprocessorDefinitions=\"";
	for (std::vector< std::wstring >::const_iterator i = solution->getDefinitions().begin(); i != solution->getDefinitions().end(); ++i)
		os << *i << L";";
	for (std::vector< std::wstring >::const_iterator i = configuration->getDefinitions().begin(); i != configuration->getDefinitions().end(); ++i)
		os << *i << L";";

	switch (configuration->getTargetFormat())
	{
	case Configuration::TfStaticLibrary:
		os << L"_LIBRARY;";
		break;

	case Configuration::TfSharedLibrary:
		os << L"_USRDLL;";
		break;
	}

	std::map< std::wstring, std::wstring >::const_iterator i2 = m_staticOptions.find(L"PreprocessorDefinitions");
	if (i2 != m_staticOptions.end())
		os << context.format(i2->second) << L";";

	os << L"\"" << Endl;

	// Precompiled headers.
	std::wstring precompiledHeader = configuration->getPrecompiledHeader();
	if (!precompiledHeader.empty())
	{
		os << L"UsePrecompiledHeader=\"2\"" << Endl;
		os << L"PrecompiledHeaderThrough=\"" << precompiledHeader << L"\"" << Endl;
	}
	else
	{
		os << L"UsePrecompiledHeader=\"0\"" << Endl;
	}

	// Static options.
	for (std::map< std::wstring, std::wstring >::const_iterator i = m_staticOptions.begin(); i != m_staticOptions.end(); ++i)
	{
		if (
			i->first == L"AdditionalOptions" ||
			i->first == L"AdditionalIncludeDirectories" ||
			i->first == L"PreprocessorDefinitions"
		)
			continue;
		os << i->first << L"=\"" << context.format(i->second) << L"\"" << Endl;
	}

	os << DecreaseIndent;
	os << L"/>" << Endl;
	return true;
}

void SolutionBuilderMsvcCompilerTool::serialize(traktor::ISerializer& s)
{
	s >> Member< bool >(L"resolvePaths", m_resolvePaths);
	s >> MemberStlMap< std::wstring, std::wstring >(L"staticOptions", m_staticOptions);
}

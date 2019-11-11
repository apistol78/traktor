#include "Core/Io/OutputStream.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStl.h"
#include "SolutionBuilder/Configuration.h"
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/Msvc/GeneratorContext.h"
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcCompilerTool.h"

namespace traktor
{
	namespace sb
	{

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
	auto i0 = m_staticOptions.find(L"AdditionalOptions");
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
	for (const auto& includePath : configuration->getIncludePaths())
	{
		std::wstring includePathRel = context.getProjectRelativePath(includePath, m_resolvePaths);
		os << includePathRel << L";";
	}

	auto i1 = m_staticOptions.find(L"AdditionalIncludeDirectories");
	if (i1 != m_staticOptions.end())
		os << context.format(i1->second) << L";";

	os << L"\"" << Endl;

	// Preprocessor definitions.
	os << L"PreprocessorDefinitions=\"";
	for (const auto& definition : configuration->getDefinitions())
		os << definition << L";";

	switch (configuration->getTargetFormat())
	{
	case Configuration::TfStaticLibrary:
		os << L"_LIBRARY;";
		break;

	case Configuration::TfSharedLibrary:
		os << L"_USRDLL;";
		break;

    default:
        break;
	}

	auto i2 = m_staticOptions.find(L"PreprocessorDefinitions");
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
	for (const auto& it : m_staticOptions)
	{
		if (
			it.first == L"AdditionalOptions" ||
			it.first == L"AdditionalIncludeDirectories" ||
			it.first == L"PreprocessorDefinitions"
		)
			continue;
		os << it.first << L"=\"" << context.format(it.second) << L"\"" << Endl;
	}

	os << DecreaseIndent;
	os << L"/>" << Endl;
	return true;
}

void SolutionBuilderMsvcCompilerTool::serialize(ISerializer& s)
{
	s >> Member< bool >(L"resolvePaths", m_resolvePaths);
	s >> MemberStlMap< std::wstring, std::wstring >(L"staticOptions", m_staticOptions);
}

	}
}

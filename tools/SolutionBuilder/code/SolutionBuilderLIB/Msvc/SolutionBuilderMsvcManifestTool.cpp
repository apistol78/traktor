/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Io/FileSystem.h>
#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Misc/String.h>
#include <Core/Log/Log.h>
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcManifestTool.h"
#include "SolutionBuilderLIB/Msvc/GeneratorContext.h"
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/ProjectDependency.h"
#include "SolutionBuilderLIB/ExternalDependency.h"
#include "SolutionBuilderLIB/Configuration.h"
#include "SolutionBuilderLIB/File.h"

using namespace traktor;

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"SolutionBuilderMsvcManifestTool", 0, SolutionBuilderMsvcManifestTool, SolutionBuilderMsvcTool)

SolutionBuilderMsvcManifestTool::SolutionBuilderMsvcManifestTool()
{
}

bool SolutionBuilderMsvcManifestTool::generate(GeneratorContext& context, Solution* solution, Project* project, Configuration* configuration, traktor::OutputStream& os) const
{
	context.set(L"MANIFEST_FILE", m_manifest);
	findManifests(context, solution, project, project->getItems());

	os << L"<Tool" << Endl;
	os << IncreaseIndent;
	os << L"Name=\"VCManifestTool\"" << Endl;
	os << L"AdditionalManifestFiles=\"" << context.get(L"MANIFEST_FILE") << L"\"" << Endl;
	os << L"EmbedManifest=\"true\"" << Endl;
	os << DecreaseIndent;
	os << L"/>" << Endl;

	return true;
}

void SolutionBuilderMsvcManifestTool::serialize(traktor::ISerializer& s)
{
	s >> Member< std::wstring >(L"manifest", m_manifest);
}

void SolutionBuilderMsvcManifestTool::findManifests(GeneratorContext& context, Solution* solution, Project* project, const RefArray< ProjectItem >& items) const
{
	Path rootPath = FileSystem::getInstance().getAbsolutePath(context.get(L"PROJECT_PATH"));

	for (RefArray< ProjectItem >::const_iterator i = items.begin(); i != items.end(); ++i)
	{
		if (const ::File* file = dynamic_type_cast< const ::File* >(*i))
		{
			std::set< Path > systemFiles;
			file->getSystemFiles(project->getSourcePath(), systemFiles);
			for (std::set< Path >::iterator j = systemFiles.begin(); j != systemFiles.end(); ++j)
			{
				if (compareIgnoreCase< std::wstring >(j->getExtension(), L"xml") == 0)
				{
					Path relativePath;
					FileSystem::getInstance().getRelativePath(
						*j,
						rootPath,
						relativePath
					);
					context.set(L"MANIFEST_FILE", relativePath.getPathName());
				}
			}
		}
		findManifests(context, solution, project, (*i)->getItems());
	}
}

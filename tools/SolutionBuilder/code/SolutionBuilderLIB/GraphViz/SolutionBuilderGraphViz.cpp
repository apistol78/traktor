/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Io/DynamicMemoryStream.h>
#include <Core/Io/FileOutputStream.h>
#include <Core/Io/FileSystem.h>
#include <Core/Io/Utf8Encoding.h>
#include <Core/Log/Log.h>
#include "SolutionBuilderLIB/ExternalDependency.h"
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/ProjectDependency.h"
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/GraphViz/SolutionBuilderGraphViz.h"

using namespace traktor;

namespace
{

void collectActiveProjects(const Project* project, std::set< const Project* >& outProjects)
{
	outProjects.insert(project);

	const RefArray< Dependency >& dependencies = project->getDependencies();
	for (RefArray< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
	{
		if (const ProjectDependency* projectDependency = dynamic_type_cast< const ProjectDependency* >(*j))
			collectActiveProjects(projectDependency->getProject(), outProjects);
		else if (const ExternalDependency* externalDependency = dynamic_type_cast< const ExternalDependency* >(*j))
			collectActiveProjects(externalDependency->getProject(), outProjects);
	}
}

}

T_IMPLEMENT_RTTI_CLASS(L"SolutionBuilderGraphViz", SolutionBuilderGraphViz, SolutionBuilder)

SolutionBuilderGraphViz::SolutionBuilderGraphViz()
:	m_skipLeafs(false)
{
}

bool SolutionBuilderGraphViz::create(const CommandLine& cmdLine)
{
	m_skipLeafs = cmdLine.hasOption(L's', L"graphviz-skipleafs");
	return true;
}

bool SolutionBuilderGraphViz::generate(Solution* solution)
{
	AlignedVector< uint8_t > buffer;
	buffer.reserve(40000);

	DynamicMemoryStream bufferStream(buffer, false, true);
	FileOutputStream os(&bufferStream, new Utf8Encoding());

	os << L"digraph G {" << Endl;
	os << IncreaseIndent;
	os << L"node [shape=box];" << Endl;

	std::set< const Project* > activeProjects;
	const RefArray< Project >& projects = solution->getProjects();
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		const Project* project = *i;
		if (project->getEnable())
			collectActiveProjects(project, activeProjects);
	}

	for (std::set< const Project* >::const_iterator i = activeProjects.begin(); i != activeProjects.end(); ++i)
	{
		const Project* project = *i;
		const RefArray< Dependency >& dependencies = project->getDependencies();
		for (RefArray< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
		{
			if (const ProjectDependency* projectDependency = dynamic_type_cast< const ProjectDependency* >(*j))
			{
				if (!m_skipLeafs || !projectDependency->getProject()->getDependencies().empty())
					os << L"\"" << project->getName() << L"\" -> \"" << projectDependency->getName() << L"\";" << Endl;
			}
			else if (const ExternalDependency* externalDependency = dynamic_type_cast< const ExternalDependency* >(*j))
			{
				if (!m_skipLeafs || !externalDependency->getProject()->getDependencies().empty())
					os << L"\"" << project->getName() << L"\" -> \"" << externalDependency->getName() << L"\";" << Endl;
			}
		}
	}

	os << DecreaseIndent;
	os << L"}" << Endl;

	os.close();

	if (!buffer.empty())
	{
		Ref< IStream > file = FileSystem::getInstance().open(
			solution->getName() + L".graphviz",
			traktor::File::FmWrite
		);
		if (!file)
			return false;
		file->write(&buffer[0], int(buffer.size()));
		file->close();
	}

	return true;
}

void SolutionBuilderGraphViz::showOptions() const
{
	traktor::log::info << L"\t-s,-graphviz-skipleafs=Skip leaf project(s)" << Endl;
}

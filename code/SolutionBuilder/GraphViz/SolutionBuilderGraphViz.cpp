/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <set>
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "SolutionBuilder/ExternalDependency.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/ProjectDependency.h"
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/GraphViz/SolutionBuilderGraphViz.h"

namespace traktor::sb
{
	namespace
	{

void collectActiveProjects(const Project* project, std::set< const Project* >& outProjects)
{
	outProjects.insert(project);
	for (auto dependency : project->getDependencies())
	{
		if (auto projectDependency = dynamic_type_cast< const ProjectDependency* >(dependency))
			collectActiveProjects(projectDependency->getProject(), outProjects);
		else if (auto externalDependency = dynamic_type_cast< const ExternalDependency* >(dependency))
			collectActiveProjects(externalDependency->getProject(), outProjects);
	}
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sb.SolutionBuilderGraphViz", SolutionBuilderGraphViz, SolutionBuilder)

bool SolutionBuilderGraphViz::create(const CommandLine& cmdLine)
{
	m_skipLeafs = cmdLine.hasOption(L's', L"graphviz-skipleafs");
	return true;
}

bool SolutionBuilderGraphViz::generate(const Solution* solution)
{
	AlignedVector< uint8_t > buffer;
	buffer.reserve(40000);

	DynamicMemoryStream bufferStream(buffer, false, true);
	FileOutputStream os(&bufferStream, new Utf8Encoding());

	os << L"digraph G {" << Endl;
	os << IncreaseIndent;
	os << L"node [shape=box];" << Endl;

	std::set< const Project* > activeProjects;
	for (auto project : solution->getProjects())
	{
		if (project->getEnable())
			collectActiveProjects(project, activeProjects);
	}

	for (auto project : activeProjects)
	{
		for (auto dependency : project->getDependencies())
		{
			if (auto projectDependency = dynamic_type_cast< const ProjectDependency* >(dependency))
			{
				if (!m_skipLeafs || !projectDependency->getProject()->getDependencies().empty())
					os << L"\"" << project->getName() << L"\" -> \"" << projectDependency->getName() << L"\";" << Endl;
			}
			else if (auto externalDependency = dynamic_type_cast< const ExternalDependency* >(dependency))
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
			File::FmWrite
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
	log::info << L"\t-s,-graphviz-skipleafs=Skip leaf project(s)" << Endl;
}

}

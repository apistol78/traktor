/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "ProjectDependency.h"

#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Project.h"

namespace traktor::sb
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sb.ProjectDependency", 3, ProjectDependency, Dependency)

ProjectDependency::ProjectDependency(Project* project)
	: m_project(project)
{
}

void ProjectDependency::setProject(Project* project)
{
	m_project = project;
}

Project* ProjectDependency::getProject() const
{
	return m_project;
}

std::wstring ProjectDependency::getName() const
{
	return m_project->getName();
}

std::wstring ProjectDependency::getLocation() const
{
	return L"";
}

bool ProjectDependency::resolve(const Path& referringSolutionPath, SolutionLoader* solutionLoader)
{
	for (auto dependency : m_project->getDependencies())
		if (!dependency->resolve(referringSolutionPath, solutionLoader))
			return false;
	return true;
}

void ProjectDependency::serialize(ISerializer& s)
{
	if (s.getVersion() >= 1)
		Dependency::serialize(s);

	s >> MemberRef< Project >(L"project", m_project);
}

}

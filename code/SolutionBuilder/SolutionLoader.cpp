/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "SolutionBuilder/Dependency.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/SolutionLoader.h"
#include "SolutionBuilder/Solution.h"
#include "Xml/XmlDeserializer.h"

namespace traktor::sb
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sb.SolutionLoader", SolutionLoader, Object)

Solution* SolutionLoader::load(const std::wstring& fileName)
{
	// Resolve absolute path to use as key.
	const std::wstring pathName = toLower(FileSystem::getInstance().getAbsolutePath(fileName).getPathName());

	// Have we already loaded the solution.
	auto it = m_solutions.find(pathName);
	if (it != m_solutions.end())
		return it->second;

	// Open solution file and deserialize solution object.
	Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmRead | File::FmMapped);
	if (!file)
		return nullptr;

	Ref< Solution > solution = xml::XmlDeserializer(file).readObject< Solution >();

	safeClose(file);

	// Resolve dependencies.
	if (solution)
	{
		for (auto project : solution->getProjects())
		{
			for (auto dependency : project->getDependencies())
			{
				if (!dependency->resolve(Path(fileName), this))
					return nullptr;
			}
		}
	}

	// Add solution to map.
	if (solution)
		m_solutions[pathName] = solution;

	return solution;
}

}

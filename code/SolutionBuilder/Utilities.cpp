/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/MemoryStream.h"
#include "Core/Io/File.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StreamCompare.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "SolutionBuilder/Configuration.h"
#include "SolutionBuilder/ExternalDependency.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/ProjectDependency.h"
#include "SolutionBuilder/Utilities.h"

namespace traktor::sb
{

bool writeFileIfMismatch(const std::wstring& fileName, const AlignedVector< uint8_t >& data)
{
	bool needToWrite = false;

	Ref< File > existingFile = FileSystem::getInstance().get(fileName);
	if (existingFile != nullptr && existingFile->getSize() == data.size())
	{
		if (!data.empty())
		{
			Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmRead | File::FmMapped);
			if (file)
			{
				MemoryStream dataStream(&data[0], data.size());
				if (!StreamCompare(file, &dataStream).execute())
					needToWrite = true;
				file->close();
			}
			else
				needToWrite = true;
		}
	}
	else
		needToWrite = true;

	if (needToWrite)
	{
		Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmWrite);
		if (!file)
			return false;

		if (!data.empty())
			file->write(&data[0], int64_t(data.size()));

		file->close();
	}

	return true;
}

void flattenIncludePaths(
	const Path& solutionPath,
	const Project* project,
	SmallSet< const Project* >& visited,
	SmallMap< std::wstring, SmallSet< std::wstring > >& outConfigurationIncludePaths,
	SmallMap< std::wstring, SmallSet< std::wstring > >& outConfigurationLibraryPaths)
{
	if (!visited.insert(project))
		return;

	const Path solutionPathAbs = FileSystem::getInstance().getAbsolutePath(solutionPath);

	for (auto configuration : project->getConfigurations())
	{
		const auto& includePaths = configuration->getIncludePaths();
		for (const auto& includePath : includePaths)
			if (!startsWith(includePath, L"`"))
			{
				const Path resolvedIncludePath = (Path(solutionPathAbs.getPathOnly()) + Path(includePath)).normalized();
				outConfigurationIncludePaths[configuration->getName()].insert(resolvedIncludePath.getPathNameOS());
			}
			else
				outConfigurationIncludePaths[configuration->getName()].insert(includePath);

		const auto& libraryPaths = configuration->getLibraryPaths();
		for (const auto& libraryPath : libraryPaths)
			if (!startsWith(libraryPath, L"`"))
			{
				const Path resolvedLibraryPath = (Path(solutionPathAbs.getPathOnly()) + Path(libraryPath)).normalized();
				outConfigurationLibraryPaths[configuration->getName()].insert(resolvedLibraryPath.getPathNameOS());
			}
			else
				outConfigurationLibraryPaths[configuration->getName()].insert(libraryPath);
	}

	for (auto dependency : project->getDependencies())
	{
		if (!dependency->getInheritIncludePaths())
			continue;

		if (auto projectDependency = dynamic_type_cast< const ProjectDependency* >(dependency))
			flattenIncludePaths(solutionPath, projectDependency->getProject(), visited, outConfigurationIncludePaths, outConfigurationLibraryPaths);
		else if (auto externalDependency = dynamic_type_cast< const ExternalDependency* >(dependency))
		{
			const Path externalSolutionPath = FileSystem::getInstance().getAbsolutePath(Path(externalDependency->getSolutionFileName()));
			flattenIncludePaths(externalSolutionPath, externalDependency->getProject(), visited, outConfigurationIncludePaths, outConfigurationLibraryPaths);
		}
	}
}

}

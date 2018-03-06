/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Misc/String.h"
#include "SolutionBuilder/Aggregation.h"
#include "SolutionBuilder/Dependency.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/SolutionLoader.h"
#include "SolutionBuilder/Solution.h"
#include "Xml/XmlDeserializer.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_CLASS(L"SolutionLoader", SolutionLoader, Object)

Solution* SolutionLoader::load(const std::wstring& fileName)
{
	// Resolve absolute path to use as key.
	std::wstring pathName = toLower(FileSystem::getInstance().getAbsolutePath(fileName).getPathName());

	// Have we already loaded the solution.
	std::map< std::wstring, Ref< Solution > >::iterator i = m_solutions.find(pathName);
	if (i != m_solutions.end())
		return i->second;

	// Open solution file and deserialize solution object.
	Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmRead);
	if (!file)
		return 0;

	Ref< Solution > solution = xml::XmlDeserializer(file).readObject< Solution >();

	file->close();

	// Resolve dependencies.
	if (solution)
	{
		const RefArray< Project >& projects = solution->getProjects();
		for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
		{
			const RefArray< Dependency >& dependencies = (*i)->getDependencies();
			for (RefArray< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
			{
				if (!(*j)->resolve(Path(fileName), this))
					return 0;
			}
		}

		const RefArray< Aggregation >& aggregations = solution->getAggregations();
		for (RefArray< Aggregation >::const_iterator i = aggregations.begin(); i != aggregations.end(); ++i)
		{
			const RefArray< Dependency >& dependencies = (*i)->getDependencies();
			for (RefArray< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
			{
				if (!(*j)->resolve(Path(fileName), this))
					return 0;
			}
		}	
	}

	// Add solution to map.
	if (solution)
		m_solutions[pathName] = solution;

	return solution;
}

	}
}

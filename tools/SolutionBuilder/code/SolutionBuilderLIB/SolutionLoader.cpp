/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Io/FileSystem.h>
#include <Core/Io/IStream.h>
#include <Core/Misc/String.h>
#include <Xml/XmlDeserializer.h>
#include "SolutionLoader.h"
#include "Solution.h"

T_IMPLEMENT_RTTI_CLASS(L"SolutionLoader", SolutionLoader, traktor::Object)

Solution* SolutionLoader::load(const std::wstring& fileName)
{
	// Resolve absolute path to use as key.
	std::wstring pathName = traktor::toLower(traktor::FileSystem::getInstance().getAbsolutePath(fileName).getPathName());

	// Have we already loaded the solution.
	std::map< std::wstring, traktor::Ref< Solution > >::iterator i = m_solutions.find(pathName);
	if (i != m_solutions.end())
		return i->second;

	// Open solution file and deserialize solution object.
	traktor::Ref< traktor::IStream > file = traktor::FileSystem::getInstance().open(fileName, traktor::File::FmRead);
	if (!file)
		return 0;

	traktor::Ref< Solution > solution = traktor::xml::XmlDeserializer(file).readObject< Solution >();

	file->close();

	// Add solution to map.
	if (solution)
		m_solutions[pathName] = solution;

	return solution;
}

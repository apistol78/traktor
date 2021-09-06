#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Misc/String.h"
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
		return nullptr;

	Ref< Solution > solution = xml::XmlDeserializer(file).readObject< Solution >();

	file->close();

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
}

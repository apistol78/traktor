#include "Core/Io/AnsiEncoding.h"
#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StringReader.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "SolutionBuilder/HeaderScanner.h"

namespace traktor
{
    namespace sb
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.sb.HeaderScanner", HeaderScanner, Object)

HeaderScanner::~HeaderScanner()
{
	for (auto cache : m_cache)
		delete cache.second;
}

void HeaderScanner::removeAllIncludePaths()
{
	m_includePaths.clear();
}

void HeaderScanner::addIncludePath(const std::wstring& includePath)
{
	m_includePaths.insert(includePath);
}

bool HeaderScanner::get(const std::wstring& fileName, const std::wstring& projectPath, SmallSet< std::wstring >& outHeaderFiles)
{
	AlignedVector< std::wstring > fileNames;
	fileNames.push_back(fileName);

	while (!fileNames.empty())
	{
		std::wstring fileName = fileNames.back();
		fileNames.pop_back();

		auto includes = scan(fileName);
		if (!includes)
			continue;

		for (auto includedFile : includes->files)
		{
			Path relativePath;
			if (FileSystem::getInstance().getRelativePath(Path(includedFile), Path(projectPath), relativePath))
			{
				std::wstring pn = relativePath.getPathName();
				if (outHeaderFiles.find(pn) != outHeaderFiles.end())
					continue;
				outHeaderFiles.insert(pn);
				fileNames.push_back(includedFile);
			}
		}
	}

	return true;
}

const HeaderScanner::Includes* HeaderScanner::scan(const std::wstring& fileName)
{
	auto it = m_cache.find(fileName);
	if (it != m_cache.end())
		return it->second;

	Includes* includes = new Includes();
	m_cache[fileName] = includes;

	if (!FileSystem::getInstance().exist(fileName))
		return includes;

	Ref< IStream > file = FileSystem::getInstance().open(fileName, traktor::File::FmRead);
	if (!file)
		return includes;

	BufferedStream bufferedFile(file);
	StringReader sr(&bufferedFile, new AnsiEncoding());

	std::wstring line;
	while (sr.readLine(line) >= 0)
	{
		line = trim(line);
		if (line.empty() || line[0] != L'#')
			continue;

		std::wstring::size_type p = line.find(L"include ");
		if (p == line.npos)
			continue;

		std::wstring dep = line.substr(p + 8);
		if (dep.length() <= 2 || dep[0] != L'\"')
			continue;

		dep = dep.substr(1);

		p = dep.find_first_of('\"');
		if (p == line.npos)
			continue;

		dep = dep.substr(0, p);

		auto filePath = Path(fileName).getPathOnly();
		if (!filePath.empty())
		{
			std::wstring dependencyName = filePath + L"/" + dep;
			if (FileSystem::getInstance().exist(dependencyName))
			{
				includes->files.insert(dependencyName);
				continue;
			}
		}

        for (auto includePath : m_includePaths)
		{
			std::wstring dependencyName = includePath + L"/" + dep;
			if (FileSystem::getInstance().exist(dependencyName))
			{
				includes->files.insert(dependencyName);
				break;
			}
		}
	}

	file->close();
	return includes;
}

    }
}

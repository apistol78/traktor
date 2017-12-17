/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/File.h"
#include "Core/Io/FileSystem.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "SolutionBuilder/File.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"File", 0, File, ProjectItem)

void File::setFileName(const std::wstring& fileName)
{
	m_fileName = fileName;
}

const std::wstring& File::getFileName() const
{
	return m_fileName;
}

void File::getSystemFiles(const Path& sourcePath, std::set< Path >& outFiles) const
{
	Path path = (FileSystem::getInstance().getAbsolutePath(sourcePath) + Path(m_fileName)).normalized();

	RefArray< traktor::File > files;
	FileSystem::getInstance().find(path, files);

	RefArray< traktor::File > directories;
	for (RefArray< traktor::File >::iterator i = files.begin(); i != files.end(); ++i)
	{
		traktor::File* file = *i;
		if (file->isDirectory() && file->getPath().getFileName() != L"." && file->getPath().getFileName() != L"..")
			directories.push_back(file);
		else if (!file->isDirectory())
		{
			if (!startsWith< std::wstring >(file->getPath().getFileName(), L"."))
				outFiles.insert(file->getPath().getPathName());
		}
	}
}

void File::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"fileName", m_fileName);
	ProjectItem::serialize(s);
}

	}
}

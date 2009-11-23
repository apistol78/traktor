#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Io/FileSystem.h>
#include <Core/Io/File.h>
#include "File.h"

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"File", 0, File, ProjectItem)

void File::setFileName(const std::wstring& fileName)
{
	m_fileName = fileName;
}

const std::wstring& File::getFileName() const
{
	return m_fileName;
}

void File::getSystemFiles(const traktor::Path& sourcePath, std::set< traktor::Path >& outFiles) const
{
	traktor::Path path = sourcePath + m_fileName;

	traktor::RefArray< traktor::File > files;
	traktor::FileSystem::getInstance().find(path, files);

	traktor::RefArray< traktor::File > directories;
	for (traktor::RefArray< traktor::File >::iterator i = files.begin(); i != files.end(); ++i)
	{
		traktor::Ref< traktor::File > file = *i;
		if (file->isDirectory() && file->getPath().getFileName() != L"." && file->getPath().getFileName() != L"..")
			directories.push_back(file);
		else if (!file->isDirectory())
			outFiles.insert(file->getPath().getPathName());
	}
}

bool File::serialize(traktor::ISerializer& s)
{
	s >> traktor::Member< std::wstring >(L"fileName", m_fileName);
	return ProjectItem::serialize(s);
}

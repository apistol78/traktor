#ifndef File_H
#define File_H

#include <set>
#include <string>
#include <Core/Io/Path.h>
#include "ProjectItem.h"

class File : public ProjectItem
{
	T_RTTI_CLASS(File)

public:
	void setFileName(const std::wstring& fileName);

	const std::wstring& getFileName() const;

	void getSystemFiles(const traktor::Path& sourcePath, std::set< traktor::Path >& outFiles) const;

	virtual bool serialize(traktor::Serializer& s);

private:
	std::wstring m_fileName;
};

#endif	// File_H

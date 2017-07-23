/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef File_H
#define File_H

#include <set>
#include <string>
#include <Core/Io/Path.h>
#include "ProjectItem.h"

class File : public ProjectItem
{
	T_RTTI_CLASS;

public:
	void setFileName(const std::wstring& fileName);

	const std::wstring& getFileName() const;

	void getSystemFiles(const traktor::Path& sourcePath, std::set< traktor::Path >& outFiles) const;

	virtual void serialize(traktor::ISerializer& s);

private:
	std::wstring m_fileName;
};

#endif	// File_H

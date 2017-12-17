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
#include "Core/Io/Path.h"
#include "SolutionBuilder/ProjectItem.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOLUTIONBUILDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sb
	{

class T_DLLCLASS File : public ProjectItem
{
	T_RTTI_CLASS;

public:
	void setFileName(const std::wstring& fileName);

	const std::wstring& getFileName() const;

	void getSystemFiles(const Path& sourcePath, std::set< Path >& outFiles) const;

	virtual void serialize(ISerializer& s);

private:
	std::wstring m_fileName;
};

	}
}

#endif	// File_H

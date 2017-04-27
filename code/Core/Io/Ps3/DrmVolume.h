/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_DrmVolume_H
#define traktor_DrmVolume_H

#include <string>
#include <np/drm_inline.h>
#include "Core/Io/IVolume.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	
class FileSystem;
	
class T_DLLCLASS DrmVolume : public IVolume
{
	T_RTTI_CLASS;

public:
	DrmVolume(const SceNpDrmKey& licensee);
	
	virtual std::wstring getDescription() const;

	virtual Ref< File > get(const Path& path);

	virtual int find(const Path& mask, RefArray< File >& out);

	virtual bool modify(const Path& fileName, uint32_t flags);
	
	virtual Ref< IStream > open(const Path& filename, uint32_t mode);
	
	virtual bool exist(const Path& filename);
	
	virtual bool remove(const Path& filename);

	virtual bool rename(const Path& fileName, const std::wstring& newName);
	
	virtual bool makeDirectory(const Path& directory);

	virtual bool removeDirectory(const Path& directory);

	virtual bool renameDirectory(const Path& directory, const std::wstring& newName);

	virtual bool setCurrentDirectory(const Path& directory);
	
	virtual Path getCurrentDirectory() const;
	
private:
	SceNpDrmKey m_licensee;
	Path m_currentDirectory;

	std::wstring getSystemPath(const Path& path) const;
};
	
}

#endif	// traktor_DrmVolume_H

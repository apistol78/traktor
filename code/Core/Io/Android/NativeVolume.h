/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_NativeVolume_H
#define traktor_NativeVolume_H

#include <string>
#include "Core/Io/IVolume.h"

namespace traktor
{

class FileSystem;

class NativeVolume : public IVolume
{
	T_RTTI_CLASS;

public:
	NativeVolume(const Path& currentDirectory);

	virtual std::wstring getDescription() const T_OVERRIDE T_FINAL;

	virtual Ref< File > get(const Path& path) T_OVERRIDE T_FINAL;

	virtual int find(const Path& mask, RefArray< File >& out) T_OVERRIDE T_FINAL;

	virtual bool modify(const Path& fileName, uint32_t flags) T_OVERRIDE T_FINAL;

	virtual Ref< IStream > open(const Path& filename, uint32_t mode) T_OVERRIDE T_FINAL;

	virtual bool exist(const Path& filename) T_OVERRIDE T_FINAL;

	virtual bool remove(const Path& filename) T_OVERRIDE T_FINAL;

	virtual bool move(const Path& fileName, const std::wstring& newName, bool overwrite) T_OVERRIDE T_FINAL;

	virtual bool copy(const Path& fileName, const std::wstring& newName, bool overwrite) T_OVERRIDE T_FINAL;

	virtual bool makeDirectory(const Path& directory) T_OVERRIDE T_FINAL;

	virtual bool removeDirectory(const Path& directory) T_OVERRIDE T_FINAL;

	virtual bool renameDirectory(const Path& directory, const std::wstring& newName) T_OVERRIDE T_FINAL;

	virtual bool setCurrentDirectory(const Path& directory) T_OVERRIDE T_FINAL;

	virtual Path getCurrentDirectory() const T_OVERRIDE T_FINAL;

	static void mountVolumes(FileSystem& fileSystem);

private:
	Path m_currentDirectory;

	std::wstring getSystemPath(const Path& path) const;
};

}

#endif	// traktor_NativeVolume_H

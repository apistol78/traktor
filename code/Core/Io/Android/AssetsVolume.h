#ifndef traktor_AssetsVolume_H
#define traktor_AssetsVolume_H

#include <string>
#include "Core/Io/IVolume.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

struct ANativeActivity;

namespace traktor
{

class FileSystem;

class T_DLLCLASS AssetsVolume : public IVolume
{
	T_RTTI_CLASS;

public:
	AssetsVolume(ANativeActivity* activity);

	virtual std::wstring getDescription() const;

	virtual Ref< File > get(const Path& path);

	virtual int find(const Path& mask, RefArray< File >& out);

	virtual bool modify(const Path& fileName, uint32_t flags);

	virtual Ref< IStream > open(const Path& filename, uint32_t mode);

	virtual bool exist(const Path& filename);

	virtual bool remove(const Path& filename);

	virtual bool move(const Path& fileName, const std::wstring& newName, bool overwrite);

	virtual bool copy(const Path& fileName, const std::wstring& newName, bool overwrite);

	virtual bool makeDirectory(const Path& directory);

	virtual bool removeDirectory(const Path& directory);

	virtual bool renameDirectory(const Path& directory, const std::wstring& newName);

	virtual bool setCurrentDirectory(const Path& directory);

	virtual Path getCurrentDirectory() const;

private:
	ANativeActivity* m_activity;
};

}

#endif	// traktor_AssetsVolume_H

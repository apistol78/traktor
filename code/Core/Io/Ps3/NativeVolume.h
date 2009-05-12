#ifndef traktor_NativeVolume_H
#define traktor_NativeVolume_H

#include <string>
#include "Core/Io/Volume.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	
class FileSystem;
	
class T_DLLCLASS NativeVolume : public Volume
{
	T_RTTI_CLASS(NativeVolume)

public:
	NativeVolume(const Path& currentDirectory);
	
	virtual std::string getDescription() const;

	virtual File* get(const Path& path);

	virtual int find(const Path& mask, RefArray< File >& out);
	
	virtual Stream* open(const Path& filename, File::Mode mode);
	
	virtual bool exist(const Path& filename);
	
	virtual bool remove(const Path& filename);
	
	virtual bool makeDirectory(const Path& directory);

	virtual bool removeDirectory(const Path& directory);

	virtual bool renameDirectory(const Path& directory, const std::string& newName);

	virtual bool setCurrentDirectory(const Path& directory);
	
	virtual Path getCurrentDirectory() const;
	
	static void mountVolumes(FileSystem& fileSystem);
	
private:
	Path m_currentDirectory;

	std::string getSystemPath(const Path& path) const;
};
	
}

#endif	// traktor_NativeVolume_H

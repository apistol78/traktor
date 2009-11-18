#ifndef traktor_zip_Volume_H
#define traktor_zip_Volume_H

#include "Core/Io/IVolume.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ZIP_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

struct zlib_filefunc_def_s;

namespace traktor
{
	namespace zip
	{
	
class T_DLLCLASS Volume : public IVolume
{
	T_RTTI_CLASS;
	
public:
	Volume(const Path& archive);

	virtual ~Volume();

	virtual std::wstring getDescription() const;

	virtual Ref< File > get(const Path& path);

	virtual int find(const Path& mask, RefArray< File >& out);

	virtual bool modify(const Path& fileName, uint32_t flags);
	
	virtual Ref< IStream > open(const Path& filename, File::Mode mode);
	
	virtual bool exist(const Path& filename);
	
	virtual bool remove(const Path& filename);
	
	virtual bool makeDirectory(const Path& directory);

	virtual bool removeDirectory(const Path& directory);

	virtual bool renameDirectory(const Path& directory, const std::wstring& newName);

	virtual bool setCurrentDirectory(const Path& directory);
	
	virtual Path getCurrentDirectory() const;

	void flush();

private:
	struct zlib_filefunc_def_s* m_zlibFF;
	Path m_archive;
	Path m_currentDirectory;
	void* m_zipFile;
	void* m_unzFile;

	bool readArchive();

	bool writeArchive();
};

	}
}

#endif	// traktor_zip_Volume_H

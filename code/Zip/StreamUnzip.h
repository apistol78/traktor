#ifndef traktor_zip_StreamUnzip_H
#define traktor_zip_StreamUnzip_H

#include <unzip.h>
#include "Core/Io/Stream.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ZIP_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace zip
	{
	
class T_DLLCLASS StreamUnzip : public Stream
{
	T_RTTI_CLASS(StreamUnzip)
	
public:
	StreamUnzip(unzFile file);

	virtual void close();

	virtual bool canRead() const;

	virtual bool canWrite() const;

	virtual bool canSeek() const;

	virtual int tell() const;

	virtual int available() const;

	virtual int seek(SeekOriginType origin, int offset);

	virtual int read(void* block, int nbytes);

	virtual int write(const void* block, int nbytes);

	virtual void flush();
	
private:
	unzFile m_file;
	unz_file_info m_info;
};

	}
}

#endif	// traktor_zip_StreamUnzip_H

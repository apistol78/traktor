#ifndef traktor_NativeStream_H
#define traktor_NativeStream_H

#include "Core/Platform.h"
#include "Core/Io/File.h"
#include "Core/Io/IStream.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class T_DLLCLASS NativeStream : public IStream
{
	T_RTTI_CLASS;

public:
	NativeStream(HANDLE hFile, uint32_t mode);
	
	virtual ~NativeStream();

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
	HANDLE m_hFile;
	uint32_t m_mode;
	mutable size_t m_fileSize;
};

}

#endif	// traktor_NativeStream_H

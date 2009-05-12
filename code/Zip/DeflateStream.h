#ifndef traktor_zip_DeflateStream_H
#define traktor_zip_DeflateStream_H

#include "Core/Heap/Ref.h"
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

class DeflateImpl;

class T_DLLCLASS DeflateStream : public Stream
{
	T_RTTI_CLASS(DeflateStream)

public:
	DeflateStream(Stream* stream, uint32_t internalBufferSize = 4096);

	virtual ~DeflateStream();

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
	Ref< DeflateImpl > m_impl;
};

	}
}

#endif	// traktor_zip_DeflateStream_H

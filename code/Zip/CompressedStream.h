#ifndef traktor_zip_CompressedStream_H
#define traktor_zip_CompressedStream_H

#include "Core/Io/IStream.h"

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

class T_DLLCLASS CompressedStream : public IStream
{
	T_RTTI_CLASS;

public:
	CompressedStream(IStream* stream);

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
	enum { CompressionBufferSize = 4096 };
	Ref< IStream > m_stream;
	mutable char m_buffer[CompressionBufferSize];
	mutable char* m_bufferEnd;
};

	}
}

#endif	// traktor_zip_CompressedStream_H

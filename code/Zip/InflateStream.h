#ifndef traktor_zip_InflateStream_H
#define traktor_zip_InflateStream_H

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

class InflateImpl;

class T_DLLCLASS InflateStream : public Stream
{
	T_RTTI_CLASS(InflateStream)

public:
	InflateStream(Stream* stream, uint32_t internalBufferSize = 4096);

	virtual ~InflateStream();

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
	Ref< InflateImpl > m_impl;
};

	}
}

#endif	// traktor_zip_InflateStream_H

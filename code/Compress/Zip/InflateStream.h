#ifndef traktor_compress_InflateStream_H
#define traktor_compress_InflateStream_H

#include "Core/Io/IStream.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_COMPRESS_ZIP_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace compress
	{

class InflateImpl;

/*! \brief Zip inflate stream.
 * \ingroup Compress
 */
class T_DLLCLASS InflateStream : public IStream
{
	T_RTTI_CLASS;

public:
	InflateStream(IStream* stream, uint32_t internalBufferSize = 4096);

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

#endif	// traktor_compress_InflateStream_H

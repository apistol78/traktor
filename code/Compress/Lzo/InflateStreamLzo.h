#ifndef traktor_compress_InflateStreamLzo_H
#define traktor_compress_InflateStreamLzo_H

#include "Core/Io/IStream.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_COMPRESS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace compress
	{

class InflateLzoImpl;

/*! \brief LZO inflate stream.
 * \ingroup Compress
 */
class T_DLLCLASS InflateStreamLzo : public IStream
{
	T_RTTI_CLASS;

public:
	InflateStreamLzo(IStream* stream, uint32_t blockSize = 64 * 1024);

	virtual ~InflateStreamLzo();

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
	Ref< InflateLzoImpl > m_impl;
};

	}
}

#endif	// traktor_compress_InflateStreamLzo_H

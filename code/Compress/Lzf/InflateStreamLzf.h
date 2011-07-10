#ifndef traktor_compress_InflateStreamLzf_H
#define traktor_compress_InflateStreamLzf_H

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

class InflateLzfImpl;

/*! \brief LZF inflate stream.
 * \ingroup Compress
 */
class T_DLLCLASS InflateStreamLzf : public IStream
{
	T_RTTI_CLASS;

public:
	InflateStreamLzf(IStream* stream, uint32_t blockSize = 64 * 1024);

	virtual ~InflateStreamLzf();

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
	Ref< InflateLzfImpl > m_impl;
};

	}
}

#endif	// traktor_compress_InflateStreamLzf_H

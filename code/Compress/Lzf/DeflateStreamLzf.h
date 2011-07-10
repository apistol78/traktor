#ifndef traktor_compress_DeflateStreamLzf_H
#define traktor_compress_DeflateStreamLzf_H

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

class DeflateLzfImpl;

/*! \brief LZF deflate stream.
 * \ingroup Compress
 */
class T_DLLCLASS DeflateStreamLzf : public IStream
{
	T_RTTI_CLASS;

public:
	DeflateStreamLzf(IStream* stream, uint32_t blockSize = 64 * 1024);

	virtual ~DeflateStreamLzf();

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
	Ref< DeflateLzfImpl > m_impl;
};

	}
}

#endif	// traktor_compress_DeflateStreamLzf_H

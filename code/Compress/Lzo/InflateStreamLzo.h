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

	virtual void close() T_OVERRIDE T_FINAL;

	virtual bool canRead() const T_OVERRIDE T_FINAL;

	virtual bool canWrite() const T_OVERRIDE T_FINAL;

	virtual bool canSeek() const T_OVERRIDE T_FINAL;

	virtual int64_t tell() const T_OVERRIDE T_FINAL;

	virtual int64_t available() const T_OVERRIDE T_FINAL;

	virtual int64_t seek(SeekOriginType origin, int64_t offset) T_OVERRIDE T_FINAL;

	virtual int64_t read(void* block, int64_t nbytes) T_OVERRIDE T_FINAL;

	virtual int64_t write(const void* block, int64_t nbytes) T_OVERRIDE T_FINAL;

	virtual void flush() T_OVERRIDE T_FINAL;

private:
	Ref< InflateLzoImpl > m_impl;
};

	}
}

#endif	// traktor_compress_InflateStreamLzo_H

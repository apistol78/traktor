#ifndef traktor_compress_InflateStreamZip_H
#define traktor_compress_InflateStreamZip_H

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

class InflateZipImpl;

/*! \brief Zip inflate stream.
 * \ingroup Compress
 */
class T_DLLCLASS InflateStreamZip : public IStream
{
	T_RTTI_CLASS;

public:
	InflateStreamZip(IStream* stream, uint32_t internalBufferSize = 4096);

	virtual ~InflateStreamZip();

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
	Ref< InflateZipImpl > m_impl;
};

	}
}

#endif	// traktor_compress_InflateStreamZip_H

#ifndef traktor_net_HttpChunkStream_H
#define traktor_net_HttpChunkStream_H

#include "Core/Io/IStream.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

/*! \brief HTTP chunk based stream.
 *
 * \note This stream is read-only and should not
 * be written to.
 */
class T_DLLCLASS HttpChunkStream : public IStream
{
	T_RTTI_CLASS;

public:
	HttpChunkStream(IStream* stream);

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
	Ref< IStream > m_stream;
	int32_t m_available;
};

	}
}

#endif	// traktor_net_HttpChunkStream_H

#ifndef traktor_net_HttpChunkStream_H
#define traktor_net_HttpChunkStream_H

#include "Core/Heap/Ref.h"
#include "Core/Io/Stream.h"

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
class T_DLLCLASS HttpChunkStream : public Stream
{
	T_RTTI_CLASS(HttpChunkStream)

public:
	HttpChunkStream(Stream* stream);

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
	Ref< Stream > m_stream;
	int32_t m_available;
};

	}
}

#endif	// traktor_net_HttpChunkStream_H

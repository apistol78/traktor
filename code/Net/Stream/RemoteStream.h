#ifndef traktor_net_RemoteStream_H
#define traktor_net_RemoteStream_H

#include "Core/Io/IStream.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Reader;
class Writer;

	namespace net
	{

class SocketAddressIPv4;
class TcpSocket;

/*! \brief
 * \ingroup Net
 */
class T_DLLCLASS RemoteStream : public IStream
{
	T_RTTI_CLASS;

public:
	static Ref< IStream > connect(const SocketAddressIPv4& addr, uint32_t id);

	virtual ~RemoteStream();

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
	Ref< TcpSocket > m_socket;
	Ref< Reader > m_reader;
	Ref< Writer > m_writer;
	uint8_t m_status;

	RemoteStream();
};

	}
}

#endif	// traktor_net_RemoteStream_H

#ifndef traktor_net_SocketStream_H
#define traktor_net_SocketStream_H

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

class Socket;

class T_DLLCLASS SocketStream : public IStream
{
	T_RTTI_CLASS;

public:
	SocketStream(Socket* socket, bool readAllowed = true, bool writeAllowed = true);

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

	void setAccess(bool readAllowed, bool writeAllowed);

private:
	mutable Ref< Socket > m_socket;
	bool m_readAllowed;
	bool m_writeAllowed;
	int m_offset;
};

	}
}

#endif	// traktor_net_SocketStream_H

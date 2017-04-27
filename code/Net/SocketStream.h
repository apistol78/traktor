/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_net_SocketStream_H
#define traktor_net_SocketStream_H

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
	namespace net
	{

class Socket;

/*! \brief Socket stream wrapper.
 * \ingroup Net
 */
class T_DLLCLASS SocketStream : public IStream
{
	T_RTTI_CLASS;

public:
	SocketStream(Socket* socket, bool readAllowed = true, bool writeAllowed = true, int32_t timeout = -1);

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

	void setAccess(bool readAllowed, bool writeAllowed);

	void setTimeout(int32_t timeout);

private:
	mutable Ref< Socket > m_socket;
	bool m_readAllowed;
	bool m_writeAllowed;
	int32_t m_timeout;
	int64_t m_offset;
};

	}
}

#endif	// traktor_net_SocketStream_H

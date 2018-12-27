/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_net_RemoteStream_H
#define traktor_net_RemoteStream_H

#include "Core/Io/IStream.h"
#include "Net/SocketAddressIPv4.h"

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

	virtual void close() override final;

	virtual bool canRead() const override final;

	virtual bool canWrite() const override final;

	virtual bool canSeek() const override final;

	virtual int64_t tell() const override final;

	virtual int64_t available() const override final;

	virtual int64_t seek(SeekOriginType origin, int64_t offset) override final;

	virtual int64_t read(void* block, int64_t nbytes) override final;

	virtual int64_t write(const void* block, int64_t nbytes) override final;

	virtual void flush() override final;

private:
	SocketAddressIPv4 m_addr;
	Ref< TcpSocket > m_socket;
	uint8_t m_status;

	RemoteStream();
};

	}
}

#endif	// traktor_net_RemoteStream_H

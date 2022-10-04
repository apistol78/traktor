#pragma once

#include "Core/Ref.h"
#include "Core/Io/IStream.h"

namespace traktor::net
{

class SocketStream;

}

namespace traktor::avalanche
{

class Client;

class ClientGetStream : public IStream
{
	T_RTTI_CLASS;

public:
	explicit ClientGetStream(Client* client, net::SocketStream* stream, int64_t blobSize);

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
	Ref< Client > m_client;
	Ref< net::SocketStream > m_stream;
	int64_t m_blobSize;
	int64_t m_offset;
};

}

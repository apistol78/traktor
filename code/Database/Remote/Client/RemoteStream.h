#ifndef traktor_db_RemoteStream_H
#define traktor_db_RemoteStream_H

#include "Core/Heap/Ref.h"
#include "Core/Io/Stream.h"

namespace traktor
{
	namespace db
	{

class Connection;

class RemoteStream : public Stream
{
	T_RTTI_CLASS(RemoteStream)

public:
	RemoteStream(Connection* connection, uint32_t handle);

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
	Ref< Connection > m_connection;
	uint32_t m_handle;
};

	}
}

#endif	// traktor_db_RemoteStream_H

#ifndef traktor_db_RemoteStream_H
#define traktor_db_RemoteStream_H

#include "Core/Io/IStream.h"

namespace traktor
{
	namespace db
	{

class Connection;

/*! \brief Remote stream.
 * \ingroup Database
 */
class RemoteStream : public IStream
{
	T_RTTI_CLASS;

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
	bool m_canRead;
	bool m_canWrite;
	bool m_canSeek;
};

	}
}

#endif	// traktor_db_RemoteStream_H

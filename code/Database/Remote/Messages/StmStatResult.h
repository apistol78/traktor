#ifndef traktor_db_StmStatResult_H
#define traktor_db_StmStatResult_H

#include "Database/Remote/IMessage.h"

namespace traktor
{
	namespace db
	{

/*! \brief
 */
class StmStatResult : public IMessage
{
	T_RTTI_CLASS(StmStatResult)

public:
	StmStatResult(bool canRead = false, bool canWrite = false, bool canSeek = false, int32_t tell = 0, int32_t available = 0);

	bool canRead() const { return m_canRead; }

	bool canWrite() const { return m_canWrite; }

	bool canSeek() const { return m_canSeek; }

	int32_t tell() const { return m_tell; }

	int32_t available() const { return m_available; }

	virtual bool serialize(Serializer& s);

private:
	bool m_canRead;
	bool m_canWrite;
	bool m_canSeek;
	int32_t m_tell;
	int32_t m_available;
};

	}
}

#endif	// traktor_db_StmStatResult_H
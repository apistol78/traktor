#ifndef traktor_db_StmStatResult_H
#define traktor_db_StmStatResult_H

#include "Database/Remote/IMessage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_REMOTE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

/*! \brief Stream status result.
 * \ingroup Database
 */
class T_DLLCLASS StmStatResult : public IMessage
{
	T_RTTI_CLASS;

public:
	StmStatResult(bool canRead = false, bool canWrite = false, bool canSeek = false, int32_t tell = 0, int32_t available = 0);

	bool canRead() const { return m_canRead; }

	bool canWrite() const { return m_canWrite; }

	bool canSeek() const { return m_canSeek; }

	int32_t tell() const { return m_tell; }

	int32_t available() const { return m_available; }

	virtual bool serialize(ISerializer& s);

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
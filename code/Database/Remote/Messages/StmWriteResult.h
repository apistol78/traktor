#ifndef traktor_db_StmWriteResult_H
#define traktor_db_StmWriteResult_H

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

/*! \brief Write block result.
 * \ingroup Database
 */
class T_DLLCLASS StmWriteResult : public IMessage
{
	T_RTTI_CLASS;

public:
	StmWriteResult(int32_t result = 0);

	uint32_t get() const { return m_result; }

	virtual bool serialize(ISerializer& s);

private:
	uint32_t m_result;
};

	}
}

#endif	// traktor_db_StmWriteResult_H
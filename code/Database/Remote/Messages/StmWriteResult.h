#ifndef traktor_db_StmWriteResult_H
#define traktor_db_StmWriteResult_H

#include "Database/Remote/IMessage.h"

namespace traktor
{
	namespace db
	{

/*! \brief Write block result.
 * \ingroup Database
 */
class StmWriteResult : public IMessage
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
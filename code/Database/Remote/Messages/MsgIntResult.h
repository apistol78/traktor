#ifndef traktor_db_MsgIntResult_H
#define traktor_db_MsgIntResult_H

#include "Database/Remote/IMessage.h"

namespace traktor
{
	namespace db
	{

/*! \brief Integer value result.
 * \ingroup Database
 */
class MsgIntResult : public IMessage
{
	T_RTTI_CLASS;

public:
	MsgIntResult(int32_t value = 0);

	uint32_t get() const { return m_value; }

	virtual bool serialize(ISerializer& s);

private:
	uint32_t m_value;
};

	}
}

#endif	// traktor_db_MsgIntResult_H
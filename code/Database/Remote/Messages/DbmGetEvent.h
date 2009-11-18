#ifndef traktor_db_DbmGetEvent_H
#define traktor_db_DbmGetEvent_H

#include "Database/Remote/IMessage.h"

namespace traktor
{
	namespace db
	{

/*! \brief Get event from bus.
 * \ingroup Database
 */
class DbmGetEvent : public IMessage
{
	T_RTTI_CLASS;

public:
	DbmGetEvent(uint32_t handle = 0);

	uint32_t getHandle() const { return m_handle; }

	virtual bool serialize(ISerializer& s);

private:
	uint32_t m_handle;
};

	}
}

#endif	// traktor_db_DbmGetEvent_H

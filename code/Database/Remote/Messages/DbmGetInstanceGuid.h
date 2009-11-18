#ifndef traktor_db_DbmGetInstanceGuid_H
#define traktor_db_DbmGetInstanceGuid_H

#include "Database/Remote/IMessage.h"

namespace traktor
{
	namespace db
	{

/*! \brief Get instance's guid.
 * \ingroup Database
 */
class DbmGetInstanceGuid : public IMessage
{
	T_RTTI_CLASS;

public:
	DbmGetInstanceGuid(uint32_t handle = 0);

	uint32_t getHandle() const { return m_handle; }

	virtual bool serialize(ISerializer& s);

private:
	uint32_t m_handle;
};

	}
}

#endif	// traktor_db_DbmGetInstanceGuid_H

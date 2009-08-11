#ifndef traktor_db_DbmSetInstanceGuid_H
#define traktor_db_DbmSetInstanceGuid_H

#include "Database/Remote/IMessage.h"
#include "Core/Guid.h"

namespace traktor
{
	namespace db
	{

/*! \brief Set instance guid.
 * \ingroup Database
 */
class DbmSetInstanceGuid : public IMessage
{
	T_RTTI_CLASS(DbmSetInstanceGuid)

public:
	DbmSetInstanceGuid(uint32_t handle = 0, const Guid& guid = Guid());

	uint32_t getHandle() const { return m_handle; }

	const Guid& getGuid() const { return m_guid; }

	virtual bool serialize(Serializer& s);

private:
	uint32_t m_handle;
	Guid m_guid;
};

	}
}

#endif	// traktor_db_DbmSetInstanceGuid_H

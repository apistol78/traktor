#ifndef traktor_db_DbmSetInstanceGuid_H
#define traktor_db_DbmSetInstanceGuid_H

#include "Database/Remote/IMessage.h"
#include "Core/Guid.h"

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

/*! \brief Set instance guid.
 * \ingroup Database
 */
class T_DLLCLASS DbmSetInstanceGuid : public IMessage
{
	T_RTTI_CLASS;

public:
	DbmSetInstanceGuid(uint32_t handle = 0, const Guid& guid = Guid());

	uint32_t getHandle() const { return m_handle; }

	const Guid& getGuid() const { return m_guid; }

	virtual void serialize(ISerializer& s);

private:
	uint32_t m_handle;
	Guid m_guid;
};

	}
}

#endif	// traktor_db_DbmSetInstanceGuid_H

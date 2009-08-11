#ifndef traktor_db_DbmOpenTransaction_H
#define traktor_db_DbmOpenTransaction_H

#include "Database/Remote/IMessage.h"

namespace traktor
{
	namespace db
	{

/*! \brief Open instance transaction.
 * \ingroup Database
 */
class DbmOpenTransaction : public IMessage
{
	T_RTTI_CLASS(DbmOpenTransaction)

public:
	DbmOpenTransaction(uint32_t handle = 0);

	uint32_t getHandle() const { return m_handle; }

	virtual bool serialize(Serializer& s);

private:
	uint32_t m_handle;
};

	}
}

#endif	// traktor_db_DbmOpenTransaction_H

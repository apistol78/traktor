#ifndef traktor_db_DbmCommitTransaction_H
#define traktor_db_DbmCommitTransaction_H

#include "Database/Remote/IMessage.h"

namespace traktor
{
	namespace db
	{

/*! \brief Commit instance transaction.
 * \ingroup Database
 */
class DbmCommitTransaction : public IMessage
{
	T_RTTI_CLASS(DbmCommitTransaction)

public:
	DbmCommitTransaction(uint32_t handle = 0);

	uint32_t getHandle() const { return m_handle; }

	virtual bool serialize(Serializer& s);

private:
	uint32_t m_handle;
};

	}
}

#endif	// traktor_db_DbmCommitTransaction_H

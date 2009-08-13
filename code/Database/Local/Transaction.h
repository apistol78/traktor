#ifndef traktor_db_Transaction_H
#define traktor_db_Transaction_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Core/Guid.h"

namespace traktor
{

class Mutex;

	namespace db
	{

class Action;
class Context;

/*! \brief Transaction
 * \ingroup Database
 */
class Transaction : public Object
{
	T_RTTI_CLASS(Transaction)

public:
	Transaction();

	virtual ~Transaction();

	bool create(const Guid& transactionGuid);

	void destroy();

	void add(Action* action);

	bool commit(Context* context);

private:
	Ref< Mutex > m_lock;
	RefArray< Action > m_actions;
	Guid m_transactionGuid;
#if defined(_DEBUG)
	bool m_locked;
#endif
};

	}
}

#endif	// traktor_db_Transaction_H

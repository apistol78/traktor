#ifndef traktor_db_Transaction_H
#define traktor_db_Transaction_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"

namespace traktor
{
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
	void add(Action* action);

	bool commit(Context* context);

private:
	RefArray< Action > m_actions;
};

	}
}

#endif	// traktor_db_Transaction_H

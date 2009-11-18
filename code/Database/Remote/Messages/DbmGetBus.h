#ifndef traktor_db_DbmGetBus_H
#define traktor_db_DbmGetBus_H

#include "Database/Remote/IMessage.h"

namespace traktor
{
	namespace db
	{

/*! \brief Get event bus.
 * \ingroup Database
 */
class DbmGetBus : public IMessage
{
	T_RTTI_CLASS;

public:
	virtual bool serialize(ISerializer& s);
};

	}
}

#endif	// traktor_db_DbmGetBus_H

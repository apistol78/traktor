#ifndef traktor_db_DbmClose_H
#define traktor_db_DbmClose_H

#include "Database/Remote/IMessage.h"

namespace traktor
{
	namespace db
	{

/*! \brief Close database.
 * \ingroup Database
 */
class DbmClose : public IMessage
{
	T_RTTI_CLASS;

public:
	virtual bool serialize(ISerializer& s);
};

	}
}

#endif	// traktor_db_DbmClose_H

#ifndef traktor_db_DbmClose_H
#define traktor_db_DbmClose_H

#include "Database/Remote/IMessage.h"

namespace traktor
{
	namespace db
	{

/*! \brief
 */
class DbmClose : public IMessage
{
	T_RTTI_CLASS(DbmClose)

public:
	virtual bool serialize(Serializer& s);
};

	}
}

#endif	// traktor_db_DbmClose_H

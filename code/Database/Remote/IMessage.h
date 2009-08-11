#ifndef traktor_db_IMessage_H
#define traktor_db_IMessage_H

#include "Core/Serialization/Serializable.h"

namespace traktor
{
	namespace db
	{

/*! \brief Network message interface.
 * \ingroup Database
 */
class IMessage : public Serializable
{
	T_RTTI_CLASS(IMessage)
};

	}
}

#endif	// traktor_db_IMessage_H

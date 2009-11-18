#ifndef traktor_db_IMessage_H
#define traktor_db_IMessage_H

#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace db
	{

/*! \brief Network message interface.
 * \ingroup Database
 */
class IMessage : public ISerializable
{
	T_RTTI_CLASS;
};

	}
}

#endif	// traktor_db_IMessage_H

#ifndef traktor_update_Item_H
#define traktor_update_Item_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UPDATE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace update
	{

/*! \brief Update item.
 * \ingroup Update
 */
class T_DLLCLASS Item : public ISerializable
{
	T_RTTI_CLASS;
};

	}
}

#endif	// traktor_update_Item_H

#ifndef traktor_IPropertyValue_H
#define traktor_IPropertyValue_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Property value base.
 * \ingroup Core
 */
class T_DLLCLASS IPropertyValue : public ISerializable
{
	T_RTTI_CLASS;
};

}

#endif	// traktor_IPropertyValue_H

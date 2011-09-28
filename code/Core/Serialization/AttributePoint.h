#ifndef traktor_AttributePoint_H
#define traktor_AttributePoint_H

#include "Core/Serialization/Attribute.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Member point attribute.
 * \ingroup Core
 */
class T_DLLCLASS AttributePoint : public Attribute
{
	T_RTTI_CLASS;
};

}

#endif	// traktor_AttributePoint_H

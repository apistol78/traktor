#ifndef traktor_AttributeDirection_H
#define traktor_AttributeDirection_H

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

/*! \brief Member direction attribute.
 * \ingroup Core
 */
class T_DLLCLASS AttributeDirection : public Attribute
{
	T_RTTI_CLASS;

public:
	AttributeDirection();

	explicit AttributeDirection(bool unit);

	bool getUnit() const { return m_unit; }

private:
	bool m_unit;
};

}

#endif	// traktor_AttributeDirection_H

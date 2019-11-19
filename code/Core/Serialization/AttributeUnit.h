#pragma once

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

enum UnitType
{
	UtUndefined,
	AuSeconds,
	AuMetres,
	AuRadians,
	AuDegrees,
	AuLumens,
	AuDecibel,
	AuNewton,
	AuKilograms,
	AuPercent,
	AuPixels,
	AuHertz
};

/*! \brief Member numeric range attribute.
 * \ingroup Core
 */
class T_DLLCLASS AttributeUnit : public Attribute
{
	T_RTTI_CLASS;

public:
	AttributeUnit(UnitType unit, bool perSecond = false);

	UnitType getUnit() const;

	bool getPerSecond() const;

private:
	UnitType m_unit;
	bool m_perSecond;
};

}


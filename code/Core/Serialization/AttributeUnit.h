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

/*! Units.
 * \ingroup Core
 */
enum class UnitType
{
	Undefined,
	Seconds,
	Metres,
	Radians,
	Degrees,
	Lumens,
	Decibel,
	Newton,
	Kilograms,
	Percent,
	Pixels,
	Hertz,
	EV
};

/*! Member unit attribute.
 * \ingroup Core
 */
class T_DLLCLASS AttributeUnit : public Attribute
{
	T_RTTI_CLASS;

public:
	explicit AttributeUnit(UnitType unit, bool perSecond = false);

	UnitType getUnit() const;

	bool getPerSecond() const;

private:
	UnitType m_unit;
	bool m_perSecond;
};

}


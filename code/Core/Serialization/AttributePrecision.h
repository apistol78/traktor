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

/*! Member precision attribute.
 * \ingroup Core
 */
class T_DLLCLASS AttributePrecision : public Attribute
{
	T_RTTI_CLASS;

public:
	enum PrecisionType
	{
		AtHalf,
		AtFull
	};

	AttributePrecision(PrecisionType precision);

	PrecisionType getPrecision() const;

private:
	PrecisionType m_precision;
};

}


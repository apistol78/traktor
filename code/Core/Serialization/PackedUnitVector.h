#ifndef traktor_net_Pack_H
#define traktor_net_Pack_H

#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Packed representation of unit vector in 16-bit data.
 * \ingroup Core
 */
class T_DLLCLASS PackedUnitVector
{
public:
	PackedUnitVector();

	PackedUnitVector(const Vector4& v);

	PackedUnitVector(uint16_t v);

	void pack(const Vector4& v);

	Vector4 unpack() const;

	uint16_t raw() const { return m_v; }

private:
	uint16_t m_v;
};

}

#endif	// traktor_net_Pack_H

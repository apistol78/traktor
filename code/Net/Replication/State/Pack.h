#ifndef traktor_net_Pack_H
#define traktor_net_Pack_H

#include "Core/Math/Vector4.h"

namespace traktor
{
	namespace net
	{

//void packUnit(const Vector4& u, uint8_t out[3]);
//
//Vector4 unpackUnit(const uint8_t u[3]);

class PackedUnitVector
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
}

#endif	// traktor_net_Pack_H

#ifndef traktor_parade_Pack_H
#define traktor_parade_Pack_H

#include "Core/Math/Vector4.h"

namespace traktor
{
	namespace parade
	{

void packUnit(const Vector4& u, uint8_t out[3]);

Vector4 unpackUnit(const uint8_t u[3]);

	}
}

#endif	// traktor_parade_Pack_H

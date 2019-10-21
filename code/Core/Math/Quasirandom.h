#pragma once

#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"

namespace traktor
{

class Random;

/*! Quasirandom methods.
 * \ingroup Core
 */
class T_DLLCLASS Quasirandom
{
public:
    static Vector2 hammersley(uint32_t i, uint32_t numSamples);

    static Vector2 hammersley(uint32_t i, uint32_t numSamples, Random& rnd);

    static Vector4 uniformHemiSphere(const Vector2& uv, const Vector4& direction);

    static Vector4 uniformCone(const Vector2& uv, const Vector4& direction, float radius);
};

}

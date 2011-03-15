#ifndef traktor_world_TSMProj_H
#define traktor_world_TSMProj_H

#include "Core/Config.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Frustum.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class WorldRenderSettings;

/*! \brief Trapezoid shadow map projection. */
void T_DLLCLASS calculateTSMProj(
	const WorldRenderSettings& settings,
	const Matrix44& viewInverse,
	const Vector4& lightPosition,
	const Vector4& lightDirection,
	const Frustum& viewFrustum,
	Matrix44& outLightView,
	Matrix44& outLightProjection,
	Matrix44& outLightSquareProjection,
	Frustum& outShadowFrustum
);

	}
}

#endif	// traktor_world_TSMProj_H

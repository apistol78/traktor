/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "World/WorldRenderSettings.h"
#include "World/SMProj/LiSPShadowProjection.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.LiSPShadowProjection", LiSPShadowProjection, IWorldShadowProjection)

void LiSPShadowProjection::calculate(
	const Matrix44& viewInverse,
	const Vector4& lightPosition,
	const Vector4& lightDirection,
	const Frustum& viewFrustum,
	const Aabb3& shadowBox,
	float shadowFarZ,
	bool quantizeProjection,
	Matrix44& outLightView,
	Matrix44& outLightProjection,
	Frustum& outShadowFrustum
) const
{
	outLightView = Matrix44::identity();
	outLightProjection = Matrix44::identity();
}

	}
}

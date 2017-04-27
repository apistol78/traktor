/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_world_UniformShadowProjection_H
#define traktor_world_UniformShadowProjection_H

#include "World/IWorldShadowProjection.h"

namespace traktor
{
	namespace world
	{

class UniformShadowProjection : public IWorldShadowProjection
{
	T_RTTI_CLASS;

public:
	UniformShadowProjection(uint32_t realShadowMapSize);

	virtual void calculate(
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
	) const T_OVERRIDE T_FINAL;

private:
	float m_realShadowMapSize;
};

	}
}

#endif	// traktor_world_UniformShadowProjection_H

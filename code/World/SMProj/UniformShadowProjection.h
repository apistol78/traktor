#pragma once

#include "World/IShadowProjection.h"

namespace traktor
{
	namespace world
	{

class UniformShadowProjection : public IShadowProjection
{
	T_RTTI_CLASS;

public:
	UniformShadowProjection(uint32_t realShadowMapSize);

	virtual void calculate(
		const Matrix44& viewInverse,
		const Vector4& lightPosition,
		const Vector4& lightDirection,
		const Frustum& viewFrustum,
		float shadowFarZ,
		bool quantizeProjection,
		Matrix44& outLightView,
		Matrix44& outLightProjection,
		Frustum& outShadowFrustum
	) const override final;

private:
	float m_realShadowMapSize;
};

	}
}


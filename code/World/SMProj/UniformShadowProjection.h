#ifndef traktor_world_UniformShadowProjection_H
#define traktor_world_UniformShadowProjection_H

#include "World/IWorldShadowProjection.h"
#include "World/WorldRenderSettings.h"

namespace traktor
{
	namespace world
	{

class WorldRenderSettings;

class UniformShadowProjection : public IWorldShadowProjection
{
	T_RTTI_CLASS;

public:
	UniformShadowProjection(
		const WorldRenderSettings& settings,
		uint32_t realShadowMapSize
	);

	virtual void calculate(
		const Matrix44& viewInverse,
		const Vector4& lightPosition,
		const Vector4& lightDirection,
		const Frustum& viewFrustum,
		const Aabb3& shadowBox,
		bool quantizeProjection,
		Matrix44& outLightView,
		Matrix44& outLightProjection,
		Matrix44& outLightSquareProjection,
		Frustum& outShadowFrustum
	) const;

private:
	WorldRenderSettings m_settings;
	float m_realShadowMapSize;
};

	}
}

#endif	// traktor_world_UniformShadowProjection_H

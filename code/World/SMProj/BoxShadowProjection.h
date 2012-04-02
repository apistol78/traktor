#ifndef traktor_world_BoxShadowProjection_H
#define traktor_world_BoxShadowProjection_H

#include "World/IWorldShadowProjection.h"
#include "World/WorldRenderSettings.h"

namespace traktor
{
	namespace world
	{

class WorldRenderSettings;

class BoxShadowProjection : public IWorldShadowProjection
{
	T_RTTI_CLASS;

public:
	BoxShadowProjection(const WorldRenderSettings& settings);

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
};

	}
}

#endif	// traktor_world_BoxShadowProjection_H

#include <limits>
#include "World/WorldRenderView.h"
#include "World/Entity/DirectionalLightEntity.h"
#include "World/Entity/LightEntityRenderer.h"
#include "World/Entity/PointLightEntity.h"

namespace traktor
{
	namespace world
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.world.LightEntityRenderer", LightEntityRenderer, IEntityRenderer)

LightEntityRenderer::LightEntityRenderer()
:	m_randomFlicker(0.0f)
{
}

const TypeInfoSet LightEntityRenderer::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< DirectionalLightEntity >());
	typeSet.insert(&type_of< PointLightEntity >());
	return typeSet;
}

void LightEntityRenderer::render(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	IWorldRenderPass& worldRenderPass,
	Entity* entity
)
{
	Light light;

	// Note: Even though we modify the light here the shadow map isn't affected until next frame.
	if (DirectionalLightEntity* directionalLightEntity = dynamic_type_cast< DirectionalLightEntity* >(entity))
	{
		Transform transform;
		directionalLightEntity->getTransform(transform);

		light.type = LtDirectional;
		light.position = transform.translation();
		light.direction = transform.rotation() * Vector4(0.0f, 1.0f, 0.0f);
		light.sunColor = directionalLightEntity->getSunColor();
		light.baseColor = directionalLightEntity->getBaseColor();
		light.shadowColor = directionalLightEntity->getShadowColor();
		light.range = Scalar(0.0f);
		light.castShadow = directionalLightEntity->getCastShadow();

		worldRenderView.addLight(light);
	}
	else if (PointLightEntity* pointLightEntity = dynamic_type_cast< PointLightEntity* >(entity))
	{
		Transform transform;
		pointLightEntity->getTransform(transform);

		Vector4 center = worldRenderView.getView() * transform.translation().xyz1();
		if (worldRenderView.getCullFrustum().inside(center, Scalar(pointLightEntity->getRange())) == Frustum::IrOutside)
			return;

		light.type = LtPoint;
		light.position = transform.translation();
		light.direction = Vector4::zero();
		light.sunColor = pointLightEntity->getSunColor();
		light.baseColor = pointLightEntity->getBaseColor();
		light.shadowColor = pointLightEntity->getShadowColor();
		light.range = Scalar(pointLightEntity->getRange());
		light.castShadow = false;

		if (pointLightEntity->getRandomFlicker() > FUZZY_EPSILON)
		{
			Scalar randomFlicker(1.0f - pointLightEntity->getRandomFlicker() * m_randomFlicker);
			light.sunColor *= randomFlicker;
			light.baseColor *= randomFlicker;
			light.shadowColor *= randomFlicker;
		}

		worldRenderView.addLight(light);
	}
}

void LightEntityRenderer::flush(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	IWorldRenderPass& worldRenderPass
)
{
	m_randomFlicker = m_random.nextFloat() * 0.5f + m_randomFlicker * 0.5f;
}

	}
}

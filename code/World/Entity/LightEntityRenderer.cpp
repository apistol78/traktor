#include <limits>
#include "Render/ITexture.h"
#include "World/WorldRenderView.h"
#include "World/Entity/DirectionalLightEntity.h"
#include "World/Entity/GodRayEntity.h"
#include "World/Entity/LightEntityRenderer.h"
#include "World/Entity/PointLightEntity.h"
#include "World/Entity/SpotLightEntity.h"

namespace traktor
{
	namespace world
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.world.LightEntityRenderer", LightEntityRenderer, IEntityRenderer)

const TypeInfoSet LightEntityRenderer::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< DirectionalLightEntity >());
	typeSet.insert(&type_of< PointLightEntity >());
	typeSet.insert(&type_of< SpotLightEntity >());
	typeSet.insert(&type_of< GodRayEntity >());
	return typeSet;
}

void LightEntityRenderer::precull(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	Entity* entity
)
{
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
	if (const DirectionalLightEntity* directionalLightEntity = dynamic_type_cast< const DirectionalLightEntity* >(entity))
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
		light.radius = Scalar(0.0f);
		light.texture = directionalLightEntity->getCloudShadowTexture();
		light.castShadow = directionalLightEntity->getCastShadow();

		worldRenderView.addLight(light);
	}
	else if (const PointLightEntity* pointLightEntity = dynamic_type_cast< const PointLightEntity* >(entity))
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
		light.radius = Scalar(0.0f);
		light.texture = 0;
		light.castShadow = false;

		if (pointLightEntity->getRandomFlicker() <= 1.0f - FUZZY_EPSILON)
		{
			Scalar randomFlicker(pointLightEntity->getRandomFlicker());
			light.sunColor *= randomFlicker;
			light.baseColor *= randomFlicker;
			light.shadowColor *= randomFlicker;
		}

		worldRenderView.addLight(light);
	}
	else if (const SpotLightEntity* spotLightEntity = dynamic_type_cast< const SpotLightEntity* >(entity))
	{
		Transform transform;
		spotLightEntity->getTransform(transform);

		//Vector4 center = worldRenderView.getView() * transform.translation().xyz1();
		//if (worldRenderView.getCullFrustum().inside(center, Scalar(spotLightEntity->getRange())) == Frustum::IrOutside)
		//	return;

		light.type = LtSpot;
		light.position = transform.translation();
		light.direction = transform.rotation() * Vector4(0.0f, 1.0f, 0.0f);
		light.sunColor = spotLightEntity->getSunColor();
		light.baseColor = spotLightEntity->getBaseColor();
		light.shadowColor = spotLightEntity->getShadowColor();
		light.range = Scalar(spotLightEntity->getRange());
		light.radius = Scalar(spotLightEntity->getRadius());
		light.texture = 0;
		light.castShadow = spotLightEntity->getCastShadow();

		worldRenderView.addLight(light);
	}
	else if (const GodRayEntity* godRayEntity = dynamic_type_cast< const GodRayEntity* >(entity))
	{
		Transform transform;
		godRayEntity->getTransform(transform);
		worldRenderView.setGodRayDirection(transform.axisZ());
	}
}

void LightEntityRenderer::flush(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	IWorldRenderPass& worldRenderPass
)
{
}

	}
}

#include <limits>
#include "Render/ITexture.h"
#include "World/WorldRenderView.h"
#include "World/Entity/DirectionalLightEntity.h"
#include "World/Entity/GodRayComponent.h"
#include "World/Entity/LightComponent.h"
#include "World/Entity/LightRenderer.h"
#include "World/Entity/PointLightEntity.h"
#include "World/Entity/SpotLightEntity.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.LightRenderer", LightRenderer, IEntityRenderer)

const TypeInfoSet LightRenderer::getRenderableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< DirectionalLightEntity >());
	typeSet.insert(&type_of< GodRayComponent >());
	typeSet.insert(&type_of< LightComponent >());
	typeSet.insert(&type_of< PointLightEntity >());
	typeSet.insert(&type_of< SpotLightEntity >());
	return typeSet;
}

void LightRenderer::render(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	Light light;

	// Note: Even though we modify the light here the shadow map isn't affected until next frame.
	if (const DirectionalLightEntity* directionalLightEntity = dynamic_type_cast< const DirectionalLightEntity* >(renderable))
	{
		Transform transform;
		directionalLightEntity->getTransform(transform);

		light.type = LtDirectional;
		light.position = transform.translation();
		light.direction = transform.rotation() * Vector4(0.0f, 1.0f, 0.0f);
		light.color = directionalLightEntity->getColor();
		light.range = Scalar(0.0f);
		light.radius = Scalar(0.0f);
		light.probe.diffuse = nullptr;
		light.probe.specular = nullptr;
		light.castShadow = directionalLightEntity->getCastShadow();

		worldRenderView.addLight(light);
	}
	else if (const PointLightEntity* pointLightEntity = dynamic_type_cast< const PointLightEntity* >(renderable))
	{
		Transform transform;
		pointLightEntity->getTransform(transform);

		Vector4 center = worldRenderView.getView() * transform.translation().xyz1();
		if (worldRenderView.getCullFrustum().inside(center, Scalar(pointLightEntity->getRange())) == Frustum::IrOutside)
			return;

		light.type = LtPoint;
		light.position = transform.translation();
		light.direction = Vector4::zero();
		light.color = pointLightEntity->getColor();
		light.range = Scalar(pointLightEntity->getRange());
		light.radius = Scalar(0.0f);
		light.probe.diffuse = nullptr;
		light.probe.specular = nullptr;
		light.castShadow = false;

		if (pointLightEntity->getRandomFlicker() <= 1.0f - FUZZY_EPSILON)
		{
			Scalar randomFlicker(pointLightEntity->getRandomFlicker());
			light.color *= randomFlicker;
		}

		worldRenderView.addLight(light);
	}
	else if (const SpotLightEntity* spotLightEntity = dynamic_type_cast< const SpotLightEntity* >(renderable))
	{
		Transform transform;
		spotLightEntity->getTransform(transform);

		//Vector4 center = worldRenderView.getView() * transform.translation().xyz1();
		//if (worldRenderView.getCullFrustum().inside(center, Scalar(spotLightEntity->getRange())) == Frustum::IrOutside)
		//	return;

		light.type = LtSpot;
		light.position = transform.translation();
		light.direction = transform.rotation() * Vector4(0.0f, 1.0f, 0.0f);
		light.color = spotLightEntity->getColor();
		light.range = Scalar(spotLightEntity->getRange());
		light.radius = Scalar(spotLightEntity->getRadius());
		light.probe.diffuse = nullptr;
		light.probe.specular = nullptr;
		light.castShadow = spotLightEntity->getCastShadow();

		worldRenderView.addLight(light);
	}

	else if (const LightComponent* lightComponent = dynamic_type_cast< const LightComponent* >(renderable))
	{
		Transform transform = lightComponent->getTransform();

		light.type = lightComponent->getLightType();
		light.position = transform.translation();
		light.direction = transform.rotation() * Vector4(0.0f, 1.0f, 0.0f);
		light.color = lightComponent->getColor();
		light.range = Scalar(lightComponent->getRange());
		light.radius = Scalar(lightComponent->getRadius());
		light.probe.diffuse = lightComponent->getProbeDiffuseTexture();
		light.probe.specular = lightComponent->getProbeSpecularTexture();
		light.castShadow = lightComponent->getCastShadow();

		worldRenderView.addLight(light);
	}

	else if (const GodRayComponent* godRayComponent = dynamic_type_cast< const GodRayComponent* >(renderable))
	{
		Transform transform = godRayComponent->getTransform();
		worldRenderView.setGodRayDirection(transform.axisZ());
	}
}

void LightRenderer::flush(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	IWorldRenderPass& worldRenderPass
)
{
}

	}
}

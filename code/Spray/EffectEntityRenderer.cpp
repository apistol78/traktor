#include "Spray/Effect.h"
#include "Spray/EffectComponent.h"
#include "Spray/EffectEntityRenderer.h"
#include "Spray/EffectEntity.h"
#include "Spray/MeshRenderer.h"
#include "Spray/PointRenderer.h"
#include "Spray/TrailRenderer.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EffectEntityRenderer", EffectEntityRenderer, world::IEntityRenderer)

EffectEntityRenderer::EffectEntityRenderer(render::IRenderSystem* renderSystem, float lod1Distance, float lod2Distance)
:	m_pointRenderer(new PointRenderer(renderSystem, lod1Distance, lod2Distance))
,	m_meshRenderer(new MeshRenderer())
,	m_trailRenderer(new TrailRenderer(renderSystem))
{
}

void EffectEntityRenderer::setLodDistances(float lod1Distance, float lod2Distance)
{
	m_pointRenderer->setLodDistances(lod1Distance, lod2Distance);
}

const TypeInfoSet EffectEntityRenderer::getRenderableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< EffectComponent >());
	typeSet.insert(&type_of< EffectEntity >());
	return typeSet;
}

void EffectEntityRenderer::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	if (EffectComponent* effectComponent = dynamic_type_cast< EffectComponent* >(renderable))
	{
		// Do we need to render anything with this technique?
		if (!effectComponent->haveTechnique(worldRenderPass.getTechnique()))
			return;

		Aabb3 boundingBox = effectComponent->getWorldBoundingBox();
		if (boundingBox.empty())
			return;

		// Early out of bounding sphere is outside of frustum.
		Vector4 center = worldRenderView.getView() * boundingBox.getCenter().xyz1();
		Scalar radius = boundingBox.getExtent().length();
		if (worldRenderView.getCullFrustum().inside(center, radius) == Frustum::IrOutside)
			return;

		Matrix44 viewInverse = worldRenderView.getView().inverse();
		Vector4 cameraPosition = viewInverse.translation().xyz1();
		Plane cameraPlane(viewInverse.axisZ(), viewInverse.translation());

		effectComponent->render(
			worldRenderPass.getTechnique(),
			cameraPosition,
			cameraPlane,
			m_pointRenderer,
			m_meshRenderer,
			m_trailRenderer
		);
	}
	else if (EffectEntity* effectEntity = dynamic_type_cast< EffectEntity* >(renderable))
	{
		// Do we need to render anything with this technique?
		if (!effectEntity->haveTechnique(worldRenderPass.getTechnique()))
			return;

		Aabb3 boundingBox = effectEntity->getWorldBoundingBox();
		if (boundingBox.empty())
			return;

		// Early out of bounding sphere is outside of frustum.
		Vector4 center = worldRenderView.getView() * boundingBox.getCenter().xyz1();
		Scalar radius = boundingBox.getExtent().length();
		if (worldRenderView.getCullFrustum().inside(center, radius) == Frustum::IrOutside)
			return;

		Matrix44 viewInverse = worldRenderView.getView().inverse();
		Vector4 cameraPosition = viewInverse.translation().xyz1();
		Plane cameraPlane(viewInverse.axisZ(), viewInverse.translation());

		effectEntity->render(
			worldRenderPass.getTechnique(),
			cameraPosition,
			cameraPlane,
			m_pointRenderer,
			m_meshRenderer,
			m_trailRenderer
		);
	}
}

void EffectEntityRenderer::flush(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
	m_pointRenderer->flush(worldContext.getRenderContext(), worldRenderPass);
	m_meshRenderer->flush(worldContext.getRenderContext(), worldRenderPass);
	m_trailRenderer->flush(worldContext.getRenderContext(), worldRenderPass);
}

	}
}

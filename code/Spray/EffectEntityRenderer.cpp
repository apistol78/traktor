#include "Spray/Effect.h"
#include "Spray/EffectComponent.h"
#include "Spray/EffectEntityRenderer.h"
#include "Spray/MeshRenderer.h"
#include "Spray/PointRenderer.h"
#include "Spray/TrailRenderer.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
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
	return makeTypeInfoSet< EffectComponent >();
}

void EffectEntityRenderer::gather(
	const world::WorldGatherContext& context,
	const Object* renderable,
	AlignedVector< world::Light >& outLights
)
{
}

void EffectEntityRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
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
}

void EffectEntityRenderer::flush(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
	m_pointRenderer->flush(context.getRenderContext(), worldRenderPass);
	m_meshRenderer->flush(context.getRenderContext(), worldRenderPass);
	m_trailRenderer->flush(context.getRenderContext(), worldRenderPass);
}

void EffectEntityRenderer::setup(const world::WorldSetupContext& context)
{
}

	}
}

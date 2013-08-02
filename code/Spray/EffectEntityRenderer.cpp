#include "Spray/EffectEntityRenderer.h"
#include "Spray/EffectEntity.h"
#include "Spray/Effect.h"
#include "Spray/MeshRenderer.h"
#include "Spray/PointRenderer.h"
#include "Spray/TrailRenderer.h"
#include "World/IWorldCulling.h"
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

const TypeInfoSet EffectEntityRenderer::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< EffectEntity >());
	return typeSet;
}

void EffectEntityRenderer::precull(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::Entity* entity
)
{
}

void EffectEntityRenderer::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	world::Entity* entity
)
{
	EffectEntity* effectEntity = checked_type_cast< EffectEntity* >(entity);

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

	if (
		worldContext.getCulling() &&
		!worldContext.getCulling()->queryAabb(boundingBox, Transform::identity())
	)
		return;

	Matrix44 viewInverse = worldRenderView.getView().inverseOrtho();
	Vector4 cameraPosition = viewInverse.translation().xyz1();
	Plane cameraPlane(viewInverse.axisZ(), viewInverse.translation());

	effectEntity->render(
		cameraPosition,
		cameraPlane,
		m_pointRenderer,
		m_meshRenderer,
		m_trailRenderer
	);
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

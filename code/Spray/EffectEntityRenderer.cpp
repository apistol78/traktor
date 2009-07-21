#include "Spray/EffectEntityRenderer.h"
#include "Spray/EffectEntity.h"
#include "Spray/Effect.h"
#include "Spray/PointRenderer.h"
#include "World/WorldContext.h"
#include "World/WorldRenderer.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EffectEntityRenderer", EffectEntityRenderer, world::IEntityRenderer)

EffectEntityRenderer::EffectEntityRenderer(render::IRenderSystem* renderSystem, float cullNearDistance, float fadeNearRange)
:	m_pointRenderer(gc_new< PointRenderer >(renderSystem, cullNearDistance, fadeNearRange))
,	m_defaltTechnique(render::getParameterHandle(L"Default"))
{
}

const TypeSet EffectEntityRenderer::getEntityTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< EffectEntity >());
	return typeSet;
}

void EffectEntityRenderer::render(
	world::WorldContext* worldContext,
	world::WorldRenderView* worldRenderView,
	world::Entity* entity
)
{
	if (worldRenderView->getTechnique() != m_defaltTechnique)
		return;

	EffectEntity* effectEntity = checked_type_cast< EffectEntity* >(entity);
	
	Aabb boundingBox = effectEntity->getWorldBoundingBox();
	if (boundingBox.empty())
		return;

	// Early out of bounding sphere is outside of frustum.
	Vector4 center = worldRenderView->getView() * boundingBox.getCenter();
	Scalar radius = boundingBox.getExtent().length();
	if (!worldRenderView->getCullFrustum().inside(center, radius))
		return;

	resource::Proxy< Effect >& effect = effectEntity->getEffect();
	if (!effect.validate())
		return;

	Matrix44 viewInverse = worldRenderView->getView().inverseOrtho();
	Plane cameraPlane(
		viewInverse.axisZ(),
		viewInverse.translation()
	);

	effectEntity->render(
		cameraPlane,
		m_pointRenderer
	);
}

void EffectEntityRenderer::flush(
	world::WorldContext* worldContext,
	world::WorldRenderView* worldRenderView
)
{
	if (worldRenderView->getTechnique() != m_defaltTechnique)
		return;

	m_pointRenderer->flush(
		worldContext->getRenderContext(),
		worldRenderView
	);
}

	}
}

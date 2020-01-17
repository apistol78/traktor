#include "Mesh/MeshComponent.h"
#include "Mesh/MeshComponentRenderer.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MeshComponentRenderer", MeshComponentRenderer, world::IEntityRenderer)

const TypeInfoSet MeshComponentRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< MeshComponent >();
}

void MeshComponentRenderer::gather(
	world::WorldContext& worldContext,
	const world::WorldRenderView& worldRenderView,
	const Object* renderable,
	AlignedVector< world::Light >& outLights
)
{
}

void MeshComponentRenderer::build(
	world::WorldContext& worldContext,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	MeshComponent* meshComponent = mandatory_non_null_type_cast< MeshComponent* >(renderable);
	meshComponent->build(worldContext, worldRenderView, worldRenderPass);
}

void MeshComponentRenderer::flush(
	world::WorldContext& worldContext,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
}

void MeshComponentRenderer::flush(world::WorldContext& worldContext)
{
}

	}
}

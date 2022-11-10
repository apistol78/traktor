#include "Mesh/MeshComponent.h"
#include "Mesh/MeshComponentRenderer.h"
#include "World/WorldGatherContext.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MeshComponentRenderer", MeshComponentRenderer, world::IEntityRenderer)

const TypeInfoSet MeshComponentRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< MeshComponent >();
}

void MeshComponentRenderer::gather(
	const world::WorldGatherContext& context,
	Object* renderable
)
{
	context.include(this, renderable);
}

void MeshComponentRenderer::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	Object* renderable
)
{
}

void MeshComponentRenderer::setup(
	const world::WorldSetupContext& context
)
{
}

void MeshComponentRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	auto meshComponent = static_cast< MeshComponent* >(renderable);
	meshComponent->build(context, worldRenderView, worldRenderPass);
}

void MeshComponentRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
}

}

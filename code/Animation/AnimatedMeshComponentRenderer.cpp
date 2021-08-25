#include "Animation/AnimatedMeshComponent.h"
#include "Animation/AnimatedMeshComponentRenderer.h"
#include "World/Entity.h"
#include "World/WorldGatherContext.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimatedMeshComponentRenderer", AnimatedMeshComponentRenderer, world::IEntityRenderer)

const TypeInfoSet AnimatedMeshComponentRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< AnimatedMeshComponent >();
}

void AnimatedMeshComponentRenderer::gather(
	const world::WorldGatherContext& context,
	Object* renderable
)
{
	auto animatedMeshComponent = static_cast< AnimatedMeshComponent* >(renderable);

	for (const auto& binding : animatedMeshComponent->getBindings())
		context.gather(binding.entity);

	context.include(this, animatedMeshComponent);
}

	}
}

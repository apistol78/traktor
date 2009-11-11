#include "World/PostProcess/PostProcessStepSetTarget.h"
#include "World/PostProcess/PostProcess.h"
#include "Core/Serialization/Serializer.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.world.PostProcessStepSetTarget", PostProcessStepSetTarget, PostProcessStep)

Ref< PostProcessStep::Instance > PostProcessStepSetTarget::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	return gc_new< InstanceSetTarget >(m_target);
}

bool PostProcessStepSetTarget::serialize(Serializer& s)
{
	return s >> Member< int32_t >(L"target", m_target);
}

// Instance

PostProcessStepSetTarget::InstanceSetTarget::InstanceSetTarget(int32_t target)
:	m_target(target)
{
}

void PostProcessStepSetTarget::InstanceSetTarget::destroy()
{
}

void PostProcessStepSetTarget::InstanceSetTarget::render(
	PostProcess* postProcess,
	render::IRenderView* renderView,
	render::ScreenRenderer* screenRenderer,
	const Frustum& viewFrustum,
	const Matrix44& projection,
	float shadowMapBias,
	float deltaTime
)
{
	postProcess->setTarget(renderView, m_target);
}

	}
}

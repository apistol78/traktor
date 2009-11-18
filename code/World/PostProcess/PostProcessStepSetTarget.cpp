#include "World/PostProcess/PostProcessStepSetTarget.h"
#include "World/PostProcess/PostProcess.h"
#include "Core/Serialization/ISerializer.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessStepSetTarget", 0, PostProcessStepSetTarget, PostProcessStep)

Ref< PostProcessStep::Instance > PostProcessStepSetTarget::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	return new InstanceSetTarget(m_target);
}

bool PostProcessStepSetTarget::serialize(ISerializer& s)
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

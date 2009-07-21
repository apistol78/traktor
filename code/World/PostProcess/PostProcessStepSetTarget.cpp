#include "World/PostProcess/PostProcessStepSetTarget.h"
#include "World/PostProcess/PostProcess.h"
#include "Core/Serialization/Serializer.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.world.PostProcessStepSetTarget", PostProcessStepSetTarget, PostProcessStep)

bool PostProcessStepSetTarget::create(PostProcess* postProcess, resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem)
{
	return true;
}

void PostProcessStepSetTarget::destroy(PostProcess* postProcess)
{
}

void PostProcessStepSetTarget::render(
	PostProcess* postProcess,
	const WorldRenderView& worldRenderView,
	render::IRenderView* renderView,
	render::ScreenRenderer* screenRenderer,
	float deltaTime
)
{
	postProcess->setTarget(renderView, m_target);
}

bool PostProcessStepSetTarget::serialize(Serializer& s)
{
	return s >> Member< uint32_t >(L"target", m_target);
}

	}
}

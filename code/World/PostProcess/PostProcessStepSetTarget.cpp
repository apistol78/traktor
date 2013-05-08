#include "World/PostProcess/PostProcessStepSetTarget.h"
#include "World/PostProcess/PostProcess.h"
#include "Core/Serialization/ISerializer.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessStepSetTarget", 0, PostProcessStepSetTarget, PostProcessStep)

Ref< PostProcessStep::Instance > PostProcessStepSetTarget::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	uint32_t width,
	uint32_t height
) const
{
	return new InstanceSetTarget(render::getParameterHandle(m_target));
}

void PostProcessStepSetTarget::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"target", m_target);
}

// Instance

PostProcessStepSetTarget::InstanceSetTarget::InstanceSetTarget(render::handle_t target)
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
	const RenderParams& params
)
{
	postProcess->setTarget(renderView, m_target);
}

	}
}

#include "World/PostProcess/PostProcessStepDiscardTarget.h"
#include "World/PostProcess/PostProcess.h"
#include "Core/Serialization/ISerializer.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessStepDiscardTarget", 0, PostProcessStepDiscardTarget, PostProcessStep)

PostProcessStepDiscardTarget::PostProcessStepDiscardTarget()
{
}

PostProcessStepDiscardTarget::PostProcessStepDiscardTarget(const std::wstring& target)
:	m_target(target)
{
}

Ref< PostProcessStep::Instance > PostProcessStepDiscardTarget::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	uint32_t width,
	uint32_t height
) const
{
	return new InstanceDiscardTarget(render::getParameterHandle(m_target));
}

void PostProcessStepDiscardTarget::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"target", m_target);
}

// Instance

PostProcessStepDiscardTarget::InstanceDiscardTarget::InstanceDiscardTarget(render::handle_t target)
:	m_target(target)
{
}

void PostProcessStepDiscardTarget::InstanceDiscardTarget::destroy()
{
}

void PostProcessStepDiscardTarget::InstanceDiscardTarget::render(
	PostProcess* postProcess,
	render::IRenderView* renderView,
	render::ScreenRenderer* screenRenderer,
	const RenderParams& params
)
{
	postProcess->discardTarget(m_target);
}

	}
}

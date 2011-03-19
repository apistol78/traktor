#include "World/PostProcess/PostProcessStepSwapTargets.h"
#include "World/PostProcess/PostProcess.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessStepSwapTargets", 0, PostProcessStepSwapTargets, PostProcessStep)

Ref< PostProcessStep::Instance > PostProcessStepSwapTargets::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	return new InstanceSwapTargets(
		render::getParameterHandle(m_destination),
		render::getParameterHandle(m_source)
	);
}

bool PostProcessStepSwapTargets::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"destination", m_destination);
	s >> Member< std::wstring >(L"source", m_source);
	return true;
}

// Instance

PostProcessStepSwapTargets::InstanceSwapTargets::InstanceSwapTargets(render::handle_t destination, render::handle_t source)
:	m_destination(destination)
,	m_source(source)
{
}

void PostProcessStepSwapTargets::InstanceSwapTargets::destroy()
{
}

void PostProcessStepSwapTargets::InstanceSwapTargets::render(
	PostProcess* postProcess,
	render::IRenderView* renderView,
	render::ScreenRenderer* screenRenderer,
	const RenderParams& params
)
{
	std::swap(
		postProcess->getTargetRef(m_destination),
		postProcess->getTargetRef(m_source)
	);
}

	}
}

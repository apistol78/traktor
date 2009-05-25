#include "World/PostProcess/PostProcessStepSwapTargets.h"
#include "World/PostProcess/PostProcess.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.world.PostProcessStepSwapTargets", PostProcessStepSwapTargets, PostProcessStep)

bool PostProcessStepSwapTargets::create(PostProcess* postProcess, render::RenderSystem* renderSystem)
{
	return true;
}

void PostProcessStepSwapTargets::destroy(PostProcess* postProcess)
{
}

void PostProcessStepSwapTargets::render(
	PostProcess* postProcess,
	const WorldRenderView& worldRenderView,
	render::RenderView* renderView,
	render::ScreenRenderer* screenRenderer,
	float deltaTime
)
{
	std::swap(
		postProcess->getTargetRef(m_destination),
		postProcess->getTargetRef(m_source)
	);
}

bool PostProcessStepSwapTargets::serialize(Serializer& s)
{
	s >> Member< uint32_t >(L"destination", m_destination);
	s >> Member< uint32_t >(L"source", m_source);
	return true;
}

	}
}

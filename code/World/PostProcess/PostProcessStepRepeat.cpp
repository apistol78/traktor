#include "World/PostProcess/PostProcessStepRepeat.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.world.PostProcessStepRepeat", PostProcessStepRepeat, PostProcessStep)

bool PostProcessStepRepeat::create(PostProcess* postProcess, resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem)
{
	return m_step->create(postProcess, resourceManager, renderSystem);
}

void PostProcessStepRepeat::destroy(PostProcess* postProcess)
{
	m_step->destroy(postProcess);
}

void PostProcessStepRepeat::render(
	PostProcess* postProcess,
	const WorldRenderView& worldRenderView,
	render::IRenderView* renderView,
	render::ScreenRenderer* screenRenderer,
	float deltaTime
)
{
	for (uint32_t i = 0; i < m_count; ++i)
	{
		m_step->render(
			postProcess,
			worldRenderView,
			renderView,
			screenRenderer,
			deltaTime
		);
	}
}

bool PostProcessStepRepeat::serialize(Serializer& s)
{
	s >> Member< uint32_t >(L"count", m_count);
	s >> MemberRef< PostProcessStep >(L"step", m_step);
	return true;
}

	}
}

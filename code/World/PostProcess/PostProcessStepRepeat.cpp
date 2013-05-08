#include "World/PostProcess/PostProcessStepRepeat.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessStepRepeat", 0, PostProcessStepRepeat, PostProcessStep)

PostProcessStepRepeat::PostProcessStepRepeat()
:	m_count(0)
{
}

Ref< PostProcessStep::Instance > PostProcessStepRepeat::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	uint32_t width,
	uint32_t height
) const
{
	Ref< Instance > instance = m_step->create(resourceManager, renderSystem, width, height);
	if (!instance)
		return 0;

	return new InstanceRepeat(m_count, instance);
}

void PostProcessStepRepeat::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"count", m_count);
	s >> MemberRef< PostProcessStep >(L"step", m_step);
}

// Instance

PostProcessStepRepeat::InstanceRepeat::InstanceRepeat(uint32_t count, Instance* instance)
:	m_count(count)
,	m_instance(instance)
{
}

void PostProcessStepRepeat::InstanceRepeat::destroy()
{
	m_instance->destroy();
}

void PostProcessStepRepeat::InstanceRepeat::render(
	PostProcess* postProcess,
	render::IRenderView* renderView,
	render::ScreenRenderer* screenRenderer,
	const RenderParams& params
)
{
	for (uint32_t i = 0; i < m_count; ++i)
	{
		m_instance->render(
			postProcess,
			renderView,
			screenRenderer,
			params
		);
	}
}

	}
}

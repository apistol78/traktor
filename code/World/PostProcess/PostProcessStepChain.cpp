#include "World/PostProcess/PostProcessStepChain.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessStepChain", 0, PostProcessStepChain, PostProcessStep)

Ref< PostProcessStep::Instance > PostProcessStepChain::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	uint32_t width,
	uint32_t height
) const
{
	RefArray< Instance > instances;
	for (RefArray< PostProcessStep >::const_iterator i = m_steps.begin(); i != m_steps.end(); ++i)
	{
		Ref< PostProcessStep::Instance > instance = (*i)->create(resourceManager, renderSystem, width, height);
		if (instance)
			instances.push_back(instance);
		else
			return 0;
	}
	return new InstanceChain(instances);
}

void PostProcessStepChain::serialize(ISerializer& s)
{
	s >> MemberRefArray< PostProcessStep >(L"steps", m_steps);
}

// Instance

PostProcessStepChain::InstanceChain::InstanceChain(const RefArray< Instance >& instances)
:	m_instances(instances)
{
}

void PostProcessStepChain::InstanceChain::destroy()
{
	for (RefArray< Instance >::iterator i = m_instances.begin(); i != m_instances.end(); ++i)
		(*i)->destroy();
}

void PostProcessStepChain::InstanceChain::render(
	PostProcess* postProcess,
	render::IRenderView* renderView,
	render::ScreenRenderer* screenRenderer,
	const RenderParams& params
)
{
	for (RefArray< Instance >::iterator i = m_instances.begin(); i != m_instances.end(); ++i)
	{
		(*i)->render(
			postProcess,
			renderView,
			screenRenderer,
			params
		);
	}
}

	}
}

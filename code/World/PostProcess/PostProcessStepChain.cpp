#include "World/PostProcess/PostProcessStepChain.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRefArray.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.world.PostProcessStepChain", PostProcessStepChain, PostProcessStep)

Ref< PostProcessStep::Instance > PostProcessStepChain::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	RefArray< Instance > instances;
	for (RefArray< PostProcessStep >::const_iterator i = m_steps.begin(); i != m_steps.end(); ++i)
	{
		Ref< PostProcessStep::Instance > instance = (*i)->create(resourceManager, renderSystem);
		if (instance)
			instances.push_back(instance);
		else
			return false;
	}
	return gc_new< InstanceChain >(cref(instances));
}

bool PostProcessStepChain::serialize(Serializer& s)
{
	return s >> MemberRefArray< PostProcessStep >(L"steps", m_steps);
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
	const Frustum& viewFrustum,
	const Matrix44& projection,
	float shadowMapBias,
	float deltaTime
)
{
	for (RefArray< Instance >::iterator i = m_instances.begin(); i != m_instances.end(); ++i)
	{
		(*i)->render(
			postProcess,
			renderView,
			screenRenderer,
			viewFrustum,
			projection,
			shadowMapBias,
			deltaTime
		);
	}
}

	}
}

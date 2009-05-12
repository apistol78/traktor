#include "World/PostProcess/PostProcessStepChain.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.world.PostProcessStepChain", PostProcessStepChain, PostProcessStep)

bool PostProcessStepChain::create(PostProcess* postProcess, render::RenderSystem* renderSystem)
{
	for (RefArray< PostProcessStep >::iterator i = m_steps.begin(); i != m_steps.end(); ++i)
	{
		if (!(*i)->create(postProcess, renderSystem))
			return false;
	}
	return true;
}

void PostProcessStepChain::destroy(PostProcess* postProcess)
{
	for (RefArray< PostProcessStep >::iterator i = m_steps.begin(); i != m_steps.end(); ++i)
		(*i)->destroy(postProcess);
}

void PostProcessStepChain::render(
	PostProcess* postProcess,
	render::RenderView* renderView,
	render::ScreenRenderer* screenRenderer,
	const Frustum& viewFrustum,
	const Matrix44& projection,
	float deltaTime
)
{
	for (RefArray< PostProcessStep >::iterator i = m_steps.begin(); i != m_steps.end(); ++i)
	{
		(*i)->render(
			postProcess,
			renderView,
			screenRenderer,
			viewFrustum,
			projection,
			deltaTime
		);
	}
}

bool PostProcessStepChain::serialize(Serializer& s)
{
	return s >> MemberRefArray< PostProcessStep >(L"steps", m_steps);
}

	}
}

#include "World/PostProcess/PostProcessStepRepeat.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.world.PostProcessStepRepeat", PostProcessStepRepeat, PostProcessStep)

bool PostProcessStepRepeat::create(PostProcess* postProcess, render::RenderSystem* renderSystem)
{
	return m_step->create(postProcess, renderSystem);
}

void PostProcessStepRepeat::destroy(PostProcess* postProcess)
{
	m_step->destroy(postProcess);
}

void PostProcessStepRepeat::render(
	PostProcess* postProcess,
	render::RenderView* renderView,
	render::ScreenRenderer* screenRenderer,
	const Frustum& viewFrustum,
	const Matrix44& projection,
	float deltaTime
)
{
	for (uint32_t i = 0; i < m_count; ++i)
	{
		m_step->render(
			postProcess,
			renderView,
			screenRenderer,
			viewFrustum,
			projection,
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

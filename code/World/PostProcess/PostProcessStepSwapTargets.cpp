#include "World/PostProcess/PostProcessStepSwapTargets.h"
#include "World/PostProcess/PostProcess.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.world.PostProcessStepSwapTargets", PostProcessStepSwapTargets, PostProcessStep)

PostProcessStep::Instance* PostProcessStepSwapTargets::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	return gc_new< InstanceSwapTargets >(m_destination, m_source);
}

bool PostProcessStepSwapTargets::serialize(Serializer& s)
{
	s >> Member< int32_t >(L"destination", m_destination);
	s >> Member< int32_t >(L"source", m_source);
	return true;
}

// Instance

PostProcessStepSwapTargets::InstanceSwapTargets::InstanceSwapTargets(int32_t destination, int32_t source)
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
	const Frustum& viewFrustum,
	const Matrix44& projection,
	float shadowMapBias,
	float deltaTime
)
{
	std::swap(
		postProcess->getTargetRef(m_destination),
		postProcess->getTargetRef(m_source)
	);
}

	}
}

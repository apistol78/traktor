#include "Amalgam/IEnvironment.h"
#include "Amalgam/Engine/Stage.h"
#include "Amalgam/Engine/StageState.h"
#include "Amalgam/Events/ReconfigureEvent.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/IRenderView.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.StageState", StageState, amalgam::IState)

StageState::StageState(
	amalgam::IEnvironment* environment,
	Stage* stage
)
:	m_environment(environment)
,	m_stage(stage)
{
}

void StageState::enter()
{
}

void StageState::leave()
{
	safeDestroy(m_stage);
}

StageState::UpdateResult StageState::update(amalgam::IStateManager* stateManager, amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info)
{
	if (m_stage->update(stateManager, control, info))
		return UrOk;
	else
		return UrExit;
}

StageState::BuildResult StageState::build(uint32_t frame, const amalgam::IUpdateInfo& info)
{
	if (m_stage->build(info, frame))
		return BrOk;
	else
		return BrFailed;
}

bool StageState::render(uint32_t frame, render::EyeType eye, const amalgam::IUpdateInfo& info)
{
	render::IRenderView* renderView = m_environment->getRender()->getRenderView();

	const Color4f clearColor(0.0f, 0.0f, 0.0f, 0.0f);
	renderView->clear(render::CfColor | render::CfDepth | render::CfStencil, &clearColor, 1.0f, 0);

	m_stage->render(eye, frame);

	return true;
}

bool StageState::take(const amalgam::IEvent* event)
{
	if (const amalgam::ReconfigureEvent* reconfigureEvent = dynamic_type_cast< const amalgam::ReconfigureEvent* >(event))
	{
		if (!reconfigureEvent->isFinished())
			m_stage->preReconfigured();
		else
			m_stage->postReconfigured();
	}
	return true;
}

	}
}

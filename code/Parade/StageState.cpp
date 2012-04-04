#include "Amalgam/IEnvironment.h"
#include "Amalgam/Actions/ReconfiguredAction.h"
#include "Parade/Stage.h"
#include "Parade/StageState.h"
#include "Render/IRenderView.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.StageState", StageState, amalgam::IState)

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
	if (m_stage)
	{
		m_stage->leave();
		m_stage->destroy();
		m_stage = 0;
	}
}

StageState::UpdateResult StageState::update(amalgam::IStateManager* stateManager, amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info)
{
	if (m_stage->update(stateManager, info))
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

	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	renderView->clear(render::CfColor | render::CfDepth | render::CfStencil, clearColor, 1.0f, 0);

	m_stage->render(eye, frame);

	return true;
}

bool StageState::take(const amalgam::IAction* action)
{
	if (const amalgam::ReconfiguredAction* reconfiguredAction = dynamic_type_cast< const amalgam::ReconfiguredAction* >(action))
	{
		m_stage->reconfigured();
	}
	return true;
}

	}
}

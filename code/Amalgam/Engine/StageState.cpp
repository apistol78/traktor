#include "Amalgam/IEnvironment.h"
#include "Amalgam/Engine/Stage.h"
#include "Amalgam/Engine/StageState.h"
#include "Amalgam/Events/ActiveEvent.h"
#include "Amalgam/Events/ReconfigureEvent.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/IRenderView.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.StageState", StageState, IState)

StageState::StageState(
	IEnvironment* environment,
	Stage* stage
)
:	m_environment(environment)
,	m_stage(stage)
{
}

void StageState::enter()
{
	log::info << Endl;
	log::info << L"-----------------------------------------------------------------------------" << Endl;
	log::info << L"Enter \"" << m_stage->getName() << L"\"" << Endl;
	log::info << L"-----------------------------------------------------------------------------" << Endl;
}

void StageState::leave()
{
	if (m_stage)
	{
		m_stage->transition();
		safeDestroy(m_stage);
	}
}

StageState::UpdateResult StageState::update(IStateManager* stateManager, const IUpdateInfo& info)
{
	if (m_stage->update(stateManager, info))
		return UrOk;
	else
		return UrExit;
}

StageState::BuildResult StageState::build(uint32_t frame, const IUpdateInfo& info)
{
	if (m_stage->build(info, frame))
		return BrOk;
	else
		return BrFailed;
}

bool StageState::render(uint32_t frame, render::EyeType eye, const IUpdateInfo& info)
{
	render::IRenderView* renderView = m_environment->getRender()->getRenderView();

	if (eye == render::EtCyclop || eye == render::EtLeft)
	{
		const Color4f clearColor(0.0f, 0.0f, 0.0f, 0.0f);
		renderView->clear(render::CfColor | render::CfDepth | render::CfStencil, &clearColor, 1.0f, 0);
	}

	m_stage->render(eye, frame);

	return true;
}

void StageState::flush()
{
	m_stage->flush();
}

bool StageState::take(const IEvent* event)
{
	if (const ReconfigureEvent* reconfigureEvent = dynamic_type_cast< const ReconfigureEvent* >(event))
	{
		if (!reconfigureEvent->isFinished())
			m_stage->preReconfigured();
		else
			m_stage->postReconfigured();
	}
	else if (const ActiveEvent* activeEvent = dynamic_type_cast< const ActiveEvent* >(event))
	{
		if (activeEvent->becameActivated())
			m_stage->resume();
		else
			m_stage->suspend();
	}
	return true;
}

	}
}

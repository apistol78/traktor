#include "Amalgam/IEnvironment.h"
#include "Amalgam/IStateManager.h"
#include "Amalgam/IUpdateControl.h"
#include "Core/Functor/Functor.h"
#include "Core/Log/Log.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Database/Database.h"
#include "Parade/LoadState.h"
#include "Parade/StageData.h"
#include "Parade/StageState.h"
#include "Render/IRenderView.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.LoadState", LoadState, amalgam::IState)

LoadState::LoadState(
	amalgam::IEnvironment* environment,
	const Guid& stageGuid,
	const Object* stageParams
)
:	m_environment(environment)
,	m_stageGuid(stageGuid)
,	m_stageParams(stageParams)
{
}

void LoadState::enter()
{
}

void LoadState::leave()
{
	m_jobLoader = 0;
}

LoadState::UpdateResult LoadState::update(amalgam::IStateManager* stateManager, amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info)
{
	if (!m_jobLoader)
	{
		// Pause entire game when loading; this prevents for example physics to be updated while loading etc.
		control.setPause(true);

		T_ASSERT (!m_nextState);
		m_jobLoader = JobManager::getInstance().add(makeFunctor(
			this,
			&LoadState::jobLoader
		));
	}
	else if (m_jobLoader->wait(50))
	{
		// Loader thread finished; unpause the game.
		control.setPause(false);

		if (m_nextState)
			stateManager->enter(m_nextState);
		else
			return UrFailed;
	}
	return UrOk;
}

LoadState::BuildResult LoadState::build(uint32_t frame, const amalgam::IUpdateInfo& info)
{
	return BrOk;
}

bool LoadState::render(uint32_t frame, render::EyeType eye, const amalgam::IUpdateInfo& info)
{
	render::IRenderView* renderView = m_environment->getRender()->getRenderView();

	const Color4f clearColor(0.0f, 0.0f, 0.0f, 0.0f);
	renderView->clear(render::CfColor | render::CfDepth | render::CfStencil, &clearColor, 1.0f, 0);

	return true;
}

bool LoadState::take(const amalgam::IAction* action)
{
	return true;
}

void LoadState::jobLoader()
{
	log::info << L"Loading begin" << Endl;

	// Load stage data directly from database.
	Ref< StageData > stageData = m_environment->getDatabase()->getObjectReadOnly< StageData >(m_stageGuid);
	if (!stageData)
		return;

	// Create stage from data; this will load further resources through resource manager.
	Ref< Stage > stage = stageData->createInstance(m_environment, m_stageParams);
	if (!stage)
		return;

	// Successfully created stage; create managing state.
	m_nextState = new StageState(m_environment, stage);

	log::info << L"Loading finished" << Endl;
}

	}
}

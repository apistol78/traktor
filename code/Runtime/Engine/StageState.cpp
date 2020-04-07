#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/TString.h"
#include "Render/IRenderView.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Runtime/IEnvironment.h"
#include "Runtime/Engine/Stage.h"
#include "Runtime/Engine/StageState.h"
#include "Runtime/Events/ActiveEvent.h"
#include "Runtime/Events/ReconfigureEvent.h"
#include "Runtime/Target/CommandEvent.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.StageState", StageState, IState)

StageState::StageState(
	IEnvironment* environment,
	Stage* stage
)
:	m_environment(environment)
,	m_stage(stage)
{
	// Create render contexts and graphs; this
	// should probably be moved so it can be
	// properly checked and also shared between
	// states.

	uint32_t frameCount = environment->getRender()->getThreadFrameQueueCount();

	m_frames.resize(frameCount);
	for (auto& frame : m_frames)
		frame.renderContext = new render::RenderContext(16 * 1024 * 1024);

	m_renderGraph = new render::RenderGraph(
		environment->getRender()->getRenderSystem()
	);
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

StageState::UpdateResult StageState::update(IStateManager* stateManager, const UpdateInfo& info)
{
	if (m_stage->update(stateManager, info))
		return UrOk;
	else
		return UrExit;
}

StageState::BuildResult StageState::build(uint32_t frame, const UpdateInfo& info)
{
	render::RenderContext* renderContext = m_frames[frame].renderContext;
	T_FATAL_ASSERT(renderContext);

	render::IRenderView* renderView = m_environment->getRender()->getRenderView();
	T_FATAL_ASSERT(renderView);

	// Render entire view.
	int32_t width = renderView->getWidth();
	int32_t height = renderView->getHeight();

	// Setup stage passes.
	if (!m_stage->setup(info, *m_renderGraph))
		return BrFailed;

	// Validate render graph.
	if (!m_renderGraph->validate())
		return BrFailed;

	// Build render context.
	renderContext->flush();
	m_renderGraph->build(renderContext, width, height);		
	return BrOk;
}

bool StageState::render(uint32_t frame, const UpdateInfo& info)
{
	render::RenderContext* renderContext = m_frames[frame].renderContext;
	T_FATAL_ASSERT(renderContext);

	render::IRenderView* renderView = m_environment->getRender()->getRenderView();
	T_FATAL_ASSERT(renderView);

	renderContext->render(renderView);
	return true;
}

bool StageState::take(const Object* event)
{
	if (auto reconfigureEvent = dynamic_type_cast< const ReconfigureEvent* >(event))
	{
		if (!reconfigureEvent->isFinished())
			m_stage->preReconfigured();
		else
			m_stage->postReconfigured();
	}
	else if (auto activeEvent = dynamic_type_cast< const ActiveEvent* >(event))
	{
		if (activeEvent->becameActivated())
			m_stage->resume();
		else
			m_stage->suspend();
	}
	else if (auto commandEvent = dynamic_type_cast< const CommandEvent* >(event))
	{
		m_stage->invokeScript(
			wstombs(commandEvent->getFunction()),
			0,
			nullptr
		);
	}
	return true;
}

	}
}

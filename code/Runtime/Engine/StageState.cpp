/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/TString.h"
#include "Core/Timer/Profiler.h"
#include "Render/IRenderView.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Runtime/IEnvironment.h"
#include "Runtime/Engine/Stage.h"
#include "Runtime/Engine/StageState.h"
#include "Runtime/Events/ActiveEvent.h"
#include "Runtime/Events/ReconfigureEvent.h"
#include "Runtime/Target/CommandEvent.h"

namespace traktor::runtime
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

	const uint32_t frameCount = environment->getRender()->getThreadFrameQueueCount();
	T_ASSERT(frameCount > 0);

	m_frames.resize(frameCount);
	for (auto& frame : m_frames)
		frame.renderContext = new render::RenderContext(16 * 1024 * 1024);

	m_renderGraph = new render::RenderGraph(
		environment->getRender()->getRenderSystem(),
		environment->getRender()->getMultiSample(),
		[](int32_t pass, int32_t level, const std::wstring& name, double start, double duration) {
			Profiler::getInstance().addEvent(name, start, duration);
		}
	);
}

StageState::~StageState()
{
	safeDestroy(m_renderGraph);
}

void StageState::enter()
{
}

void StageState::leave()
{
	if (m_stage)
	{
		m_stage->transition();
		safeDestroy(m_stage);
	}
	safeDestroy(m_renderGraph);
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
	const int32_t width = renderView->getWidth();
	const int32_t height = renderView->getHeight();

	// Setup stage passes.
	{
		T_PROFILER_SCOPE(L"Stage setup");
		if (!m_stage->setup(info, *m_renderGraph))
			return BrFailed;
	}

	// Validate render graph.
	{
		T_PROFILER_SCOPE(L"Stage validate");
		if (!m_renderGraph->validate())
			return BrFailed;
	}

	// Build render context.
	{
		T_PROFILER_SCOPE(L"Stage build");
		renderContext->flush();
		m_renderGraph->build(renderContext, width, height);		
	}
	return BrOk;
}

bool StageState::render(uint32_t frame, const UpdateInfo& info)
{
	T_PROFILER_SCOPE(L"Stage render");

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
		{
			// Ensure no pending render contexts before reconfiguration in case any
			// render resource is destroyed.
			for (auto& frame : m_frames)
				frame.renderContext->flush();

			m_stage->preReconfigured();
		}
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

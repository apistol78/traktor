/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/IRuntimePlugin.h"
#include "Runtime/IState.h"
#include "Runtime/Events/ActiveEvent.h"
#include "Runtime/Events/HotReloadEvent.h"
#include "Runtime/Events/ReconfigureEvent.h"
#include "Runtime/Impl/Application.h"
#include "Runtime/Impl/AudioServer.h"
#include "Runtime/Impl/Environment.h"
#include "Runtime/Impl/StateManager.h"
#include "Runtime/Impl/InputServer.h"
#include "Runtime/Impl/OnlineServer.h"
#include "Runtime/Impl/PhysicsServer.h"
#include "Runtime/Impl/RenderServerDefault.h"
#include "Runtime/Impl/RenderServerEmbedded.h"
#include "Runtime/Impl/ResourceServer.h"
#include "Runtime/Impl/ScriptServer.h"
#include "Runtime/Impl/WorldServer.h"
#include "Runtime/Target/IRemoteEvent.h"
#include "Runtime/Target/TargetManagerConnection.h"
#include "Runtime/Target/TargetProfilerDictionary.h"
#include "Runtime/Target/TargetProfilerEvents.h"
#include "Core/Platform.h"
#include "Core/Library/Library.h"
#include "Core/Log/Log.h"
#include "Core/Math/Float.h"
#include "Core/Math/MathUtils.h"
#include "Core/Memory/Alloc.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Core/System/OS.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/JobManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Timer/Profiler.h"
#include "Database/Database.h"
#include "Database/Events/EvtInstanceCommitted.h"
#include "Online/ISessionManager.h"
#include "Net/BidirectionalObjectTransport.h"
#include "Physics/PhysicsManager.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Resource/IResourceManager.h"
#include "Script/IScriptManager.h"
#include "World/EntityEventManager.h"

namespace traktor::runtime
{
	namespace
	{

const int32_t c_databasePollInterval = 5;

class TargetPerformanceListener : public RefCountImpl< Profiler::IReportListener >
{
public:
	explicit TargetPerformanceListener(TargetManagerConnection* targetManagerConnection)
	:	m_targetManagerConnection(targetManagerConnection)
	{
	}

	virtual void reportProfilerDictionary(const SmallMap< uint16_t, std::wstring >& dictionary) override final
	{
		auto transport = m_targetManagerConnection->getTransport();
		if (transport)
		{
			TargetProfilerDictionary targetProfilerDictionary(dictionary);
			transport->send(&targetProfilerDictionary);
		}		
	}

	virtual void reportProfilerEvents(double currentTime, const Profiler::eventQueue_t& events) override final
	{
		auto transport = m_targetManagerConnection->getTransport();
		if (transport)
		{
			TargetProfilerEvents targetProfilerEvents(currentTime, events);
			transport->send(&targetProfilerEvents);
		}
	}

private:
	Ref< TargetManagerConnection > m_targetManagerConnection;
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.Application", Application, IApplication)

bool Application::create(
	const PropertyGroup* defaultSettings,
	PropertyGroup* settings,
	const SystemApplication& sysapp,
	const SystemWindow* syswin
)
{
	// Establish target manager connection is launched from the Editor.
	const std::wstring targetManagerHost = settings->getProperty< std::wstring >(L"Runtime.TargetManager/Host");
	const int32_t targetManagerPort = settings->getProperty< int32_t >(L"Runtime.TargetManager/Port");
	const Guid targetManagerId = Guid(settings->getProperty< std::wstring >(L"Runtime.TargetManager/Id"));
	if (!targetManagerHost.empty() && targetManagerPort && targetManagerId.isValid())
	{
		m_targetManagerConnection = new TargetManagerConnection();
		if (!m_targetManagerConnection->connect(targetManagerHost, targetManagerPort, targetManagerId))
		{
			log::warning << L"Unable to connect to target manager at \"" << targetManagerHost << L"\"; unable to debug" << Endl;
			m_targetManagerConnection = 0;
		}
		if (m_targetManagerConnection)
			Profiler::getInstance().setListener(new TargetPerformanceListener(m_targetManagerConnection));
	}

	// Load dependent modules.
#if !defined(T_STATIC)
	const auto modules = defaultSettings->getProperty< SmallSet< std::wstring > >(L"Runtime.Modules");
	for (const auto& module : modules)
	{
		Ref< Library > library = new Library();
		if (library->open(module))
			m_libraries.push_back(library);
		else
			log::warning << L"Unable to load module \"" << module << L"\"" << Endl;
	}
#endif

	// Simulation setup.
	m_maxSimulationUpdates = defaultSettings->getProperty< int32_t >(L"Runtime.MaxSimulationUpdates", 8);
	m_maxSimulationUpdates = max(m_maxSimulationUpdates, 1);

	m_stateManager = new StateManager();

	// Database
	T_DEBUG(L"Creating database...");
	m_database = new db::Database ();
	const std::wstring connectionString = settings->getProperty< std::wstring >(L"Runtime.Database");
	if (!m_database->open(connectionString))
	{
		log::error << L"Application failed; unable to open database \"" << connectionString << L"\"" << Endl;
		return false;
	}

	// Online
	if (settings->getProperty(L"Online.Type"))
	{
		T_DEBUG(L"Creating online server...");
		m_onlineServer = new OnlineServer();
		if (!m_onlineServer->create(defaultSettings, settings, m_database))
			return false;
	}

	// Render
	T_DEBUG(L"Creating render server...");
	if (syswin)
	{
		Ref< RenderServerEmbedded > renderServer = new RenderServerEmbedded();
		if (!renderServer->create(defaultSettings, settings, sysapp, *syswin))
			return false;
		m_renderServer = renderServer;
	}
	else
	{
		Ref< RenderServerDefault > renderServer = new RenderServerDefault();
		if (!renderServer->create(defaultSettings, settings, sysapp))
			return false;
		m_renderServer = renderServer;
	}

	// Resource
	T_DEBUG(L"Creating resource server...");
	m_resourceServer = new ResourceServer();
	if (!m_resourceServer->create(settings, m_database))
		return false;

	// Input
	T_DEBUG(L"Creating input server...");
	m_inputServer = new InputServer();
	if (!m_inputServer->create(
		defaultSettings,
		settings,
		m_database,
		sysapp,
		syswin ? *syswin : m_renderServer->getRenderView()->getSystemWindow()
	))
		return false;

	// Physics
	if (settings->getProperty(L"Physics.Type"))
	{
		T_DEBUG(L"Creating physics server...");
		m_physicsServer = new PhysicsServer();
		if (!m_physicsServer->create(defaultSettings, settings))
			return false;
	}

	// Script
	if (settings->getProperty(L"Script.Type"))
	{
		T_DEBUG(L"Creating script server...");
		m_scriptServer = new ScriptServer();

		const bool attachDebugger = settings->getProperty< bool >(L"Script.AttachDebugger", false);
		const bool attachProfiler = settings->getProperty< bool >(L"Script.AttachProfiler", false);

		if ((attachDebugger || attachProfiler) && m_targetManagerConnection)
		{
			if (!m_scriptServer->create(
				defaultSettings,
				settings,
				attachDebugger,
				attachProfiler,
				m_inputServer->getInputSystem(),
				m_targetManagerConnection->getTransport()
			))
				return false;
		}
		else
		{
			if (!m_scriptServer->create(
				defaultSettings,
				settings,
				false,
				false,
				m_inputServer->getInputSystem(),
				nullptr
			))
				return false;
		}
	}

	// World
	T_DEBUG(L"Creating world server...");
	if (settings->getProperty(L"World.Type"))
	{
		m_worldServer = new WorldServer();
		if (!m_worldServer->create(defaultSettings, settings, m_renderServer, m_resourceServer))
			return false;
	}

	// Audio
	T_DEBUG(L"Creating audio server...");
	if (settings->getProperty(L"Audio.Type"))
	{
		m_audioServer = new AudioServer();
		if (!m_audioServer->create(settings, sysapp))
			return false;
	}

	// Environment
	T_DEBUG(L"Creating environment...");
	m_environment = new Environment(
		settings,
		m_database,
		&m_updateControl,
		m_audioServer,
		m_inputServer,
		m_onlineServer,
		m_physicsServer,
		m_renderServer,
		m_resourceServer,
		m_scriptServer,
		m_worldServer
	);

	// Resource factories
	T_DEBUG(L"Creating resource factories...");
	if (m_audioServer)
		m_audioServer->createResourceFactories(m_environment);
	if (m_inputServer)
		m_inputServer->createResourceFactories(m_environment);
	if (m_physicsServer)
		m_physicsServer->createResourceFactories(m_environment);
	if (m_renderServer)
		m_renderServer->createResourceFactories(m_environment);
	if (m_resourceServer)
		m_resourceServer->createResourceFactories(m_environment);
	if (m_scriptServer)
		m_scriptServer->createResourceFactories(m_environment);
	if (m_worldServer)
		m_worldServer->createResourceFactories(m_environment);

	// Entity factories.
	T_DEBUG(L"Creating entity factories...");
	if (m_worldServer)
		m_worldServer->createEntityFactories(m_environment);
	if (m_worldServer && m_physicsServer)
		m_physicsServer->createEntityFactories(m_environment);

	// Entity renderers.
	T_DEBUG(L"Creating entity renderers...");
	if (m_worldServer)
		m_worldServer->createEntityRenderers(m_environment);

	// Setup voice chat feature.
	if (m_onlineServer && m_audioServer)
		m_onlineServer->setupVoice(m_audioServer);

	// Database monitoring thread.
	if (settings->getProperty< bool >(L"Runtime.DatabaseThread", false))
	{
		T_DEBUG(L"Creating database monitoring thread...");
		m_threadDatabase = ThreadManager::getInstance().create([this](){ threadDatabase(); }, L"Database events");
		if (m_threadDatabase)
			m_threadDatabase->start(Thread::Highest);
	}

	// Initial, startup, state.
	T_DEBUG(L"Creating plugins...");
	for (const auto pluginType : type_of< IRuntimePlugin >().findAllOf(false))
	{
		Ref< IRuntimePlugin > plugin = dynamic_type_cast<IRuntimePlugin*>(pluginType->createInstance());
		if (!plugin)
			continue;

		if (!plugin->create(m_environment))
		{
			log::error << L"Application failed; plugin \"" << type_name(plugin) << L"\" failed to initialize." << Endl;
			return false;
		}

		T_DEBUG(L"Plugin \"" << type_name(plugin) << L"\" initialized successfully.");
		m_plugins.push_back(plugin);
	}

	// Use same simulation rate as close to a multiple of display rate as possible by default.
	if (m_renderServer)
	{
		const float refreshRate = m_renderServer->getRefreshRate();
		if (refreshRate > 0 && refreshRate < 60)
		{
			const float k = (int32_t)(60.0f / refreshRate + 0.5f);
			m_updateControl.m_simulationFrequency = refreshRate * k;
			log::info << L"Display refresh rate " << refreshRate << L" Hz; using " << m_updateControl.m_simulationFrequency << L" Hz simulation rate." << Endl;
		}
	}

	Ref< IState > state;
	for (const auto plugin : m_plugins)
	{
		if ((state = plugin->createInitialState(m_environment)) != nullptr)
			break;
	}
	if (!state)
	{
		log::error << L"Application failed; unable to create initial state (scanned " << (int32_t)m_plugins.size() << L" plugins)." << Endl;
		return false;
	}

	log::info << L"Application started successfully; enter initial state..." << Endl;

	m_timer.reset();
	m_stateManager->enter(state);

	log::info << L"Initial state ready; enter main loop..." << Endl;

	// Create render thread if enabled and we're running on a multi core system.
	if (
		OS::getInstance().getCPUCoreCount() >= 2 &&
		settings->getProperty< bool >(L"Runtime.RenderThread", true)
	)
	{
		m_threadRender = ThreadManager::getInstance().create([this](){ threadRender(); }, L"Render");
		if (m_threadRender)
		{
#if defined(__IOS__)
			// Create iOS thread at highest priority to indicate for scheduler to preferably
			// use "performance" cores.
			m_threadRender->start(Thread::Highest);
#else
			m_threadRender->start(Thread::Above);
#endif
			if (m_signalRenderFinish.wait(60 * 1000))
				log::info << L"Render thread started successfully." << Endl;
			else
				log::warning << L"Unable to synchronize render thread." << Endl;
		}
	}
	else
		log::info << L"Using single threaded rendering." << Endl;

	m_pauseYield = settings->getProperty< bool >(L"Runtime.PauseYield", false);
	m_settings = settings;
	return true;
}

void Application::destroy()
{
	Profiler::getInstance().setListener(nullptr);

	if (m_threadRender)
	{
		m_threadRender->stop();
		ThreadManager::getInstance().destroy(m_threadRender);
		m_threadRender = nullptr;
	}

	if (m_threadDatabase)
	{
		m_threadDatabase->stop();
		ThreadManager::getInstance().destroy(m_threadDatabase);
		m_threadDatabase = nullptr;
	}

	JobManager::getInstance().stop();

	safeDestroy(m_stateManager);

	// Cleanup all pending garbage in script land.
	if (m_scriptServer)
		m_scriptServer->cleanup(true);

	// Shutdown plugins.
	for (auto plugin : m_plugins)
		plugin->destroy(m_environment);
	m_plugins.clear();

	// Destroy environment servers.
	safeDestroy(m_resourceServer);
	safeDestroy(m_scriptServer);
	safeDestroy(m_audioServer);
	safeDestroy(m_worldServer);
	safeDestroy(m_physicsServer);
	safeDestroy(m_inputServer);
	safeDestroy(m_renderServer);
	safeDestroy(m_onlineServer);
	m_environment = nullptr;

	// Close database.
	safeClose(m_database);

	// Unload libraries.
	for (auto library : m_libraries)
		library->detach();
	m_libraries.clear();
}

bool Application::update()
{
	T_ANONYMOUS_VAR(Acquire< TicketLock >)(m_lockUpdate);
	T_PROFILER_SCOPE(L"Application update");
	Ref< IState > currentState;

	// Update target manager connection.
	if (m_targetManagerConnection && !m_targetManagerConnection->update())
	{
		log::warning << L"Connection to target manager lost; terminating application..." << Endl;
		return false;
	}

	// Update render server.
	RenderServer::UpdateResult updateResult;
	{
		T_PROFILER_SCOPE(L"Application update - Render server");
		if ((updateResult = m_renderServer->update(m_settings)) == RenderServer::UrTerminate)
			return false;
	}

	// Perform reconfiguration if required.
	if (updateResult == RenderServer::UrReconfigure || m_environment->shouldReconfigure())
	{
		// Synchronize rendering thread first as renderer might be reconfigured.
		T_ANONYMOUS_VAR(Acquire< TicketLock >)(m_lockRender);

		// Emit action in current state as we're about to reconfigured servers.
		// Also ensure state are flushed before reconfiguring.
		if ((currentState = m_stateManager->getCurrent()) != nullptr)
		{
			const ReconfigureEvent configureEvent(false, 0);
			currentState->take(&configureEvent);
		}

		// Execute configuration on all servers.
		const int32_t result = m_environment->executeReconfigure();
		if (result == CrFailed)
		{
			log::error << L"Failed to reconfigure application; cannot continue." << Endl;
			return false;
		}

		// Emit action in current state as we've successfully reconfigured servers.
		if ((currentState = m_stateManager->getCurrent()) != nullptr)
		{
			const ReconfigureEvent configureEvent(true, result);
			currentState->take(&configureEvent);
		}
	}

	// Update online session manager.
	if (m_onlineServer)
	{
		online::ISessionManager* sessionManager = m_onlineServer->getSessionManager();
		if (sessionManager)
		{
			T_PROFILER_SCOPE(L"Application update - Session manager");
			sessionManager->update();
		}
	}

	// Handle state transitions.
	if (m_stateManager->beginTransition())
	{
		// Transition begun; need to synchronize rendering thread as
		// it require current state.
		if (
			m_threadRender &&
#	if !defined(_DEBUG)
			!m_signalRenderFinish.wait(1000)
#	else
			!m_signalRenderFinish.wait()
#	endif
		)
		{
			log::error << L"Unable to synchronize render thread; render thread seems to be stuck." << Endl;
			return false;
		}

		// Ensure state transition is safe.
		{
			T_ANONYMOUS_VAR(Acquire< TicketLock >)(m_lockRender);
			T_FATAL_ASSERT (m_stateRender == nullptr);

			// Leave current state.
			T_ASSERT_M (m_stateManager->getCurrent() == nullptr || m_stateManager->getCurrent()->getReferenceCount() == 1, L"Current state must have one reference only");
			log::debug << L"Leaving state \"" << type_name(m_stateManager->getCurrent()) << L"\"..." << Endl;
			m_stateManager->leaveCurrent();

			// Cleanup script garbage.
			log::debug << L"Performing full garbage collect cycle..." << Endl;
			if (m_scriptServer)
				m_scriptServer->cleanup(true);

			// Cleanup resources used by former state.
			log::debug << L"Cleaning resident resources..." << Endl;
			m_resourceServer->performCleanup();

			// Reset time data.
			m_updateInfo.m_frameDeltaTime = 1.0 / m_updateControl.m_simulationFrequency;
			m_updateInfo.m_simulationTime = 0.0;
			m_updateInfo.m_stateTime = 0.0;

			// Enter new state.
			T_ASSERT_M (m_stateManager->getNext() && m_stateManager->getNext()->getReferenceCount() == 1, L"Next state must exist and have one reference only");
			log::debug << L"Enter state \"" << type_name(m_stateManager->getNext()) << L"\"..." << Endl;
			m_stateManager->enterNext();

			// Assume state's active from start.
			m_renderViewActive = true;
			log::debug << L"State transition complete." << Endl;

			// Reset timer's delta time as this will be very high
			// after a state change.
			m_timer.getDeltaTime();
		}
	}

	double gcDuration = 0.0;
	double updateDuration = 0.0;
	double physicsDuration = 0.0;
	double inputDuration = 0.0;
	double updateInterval = 0.0;
	int32_t updateCount = 0;

	if ((currentState = m_stateManager->getCurrent()) != nullptr)
	{
#if !defined(__ANDROID__) && !defined(__IOS__)
		// Check render active state; notify application when changes.
		const bool renderViewActive = m_renderServer->getRenderView()->isActive() && !m_renderServer->getRenderView()->isMinimized();
		if (renderViewActive != m_renderViewActive)
		{
			const ActiveEvent activeEvent(renderViewActive);
			currentState->take(&activeEvent);
			m_renderViewActive = renderViewActive;

			// If not active we can also throttle down process's priority which might
			// help updating resources from editor if running on the same machine.
			OS::getInstance().setOwnProcessPriorityBias(renderViewActive ? 0 : -1);
		}
#endif

		// Determine if input should be enabled.
		bool inputEnabled = m_renderViewActive;
		if (m_onlineServer)
			inputEnabled &= !m_onlineServer->getSessionManager()->requireUserAttention();

		// Measure delta time.
		const double deltaTime = m_timer.getDeltaTime();
		const double deltaTimeDiff = clamp(deltaTime - m_updateInfo.m_frameDeltaTime, -0.01, 0.01);
		m_updateInfo.m_frameDeltaTime = lerp(m_updateInfo.m_frameDeltaTime, m_updateInfo.m_frameDeltaTime + deltaTimeDiff, 0.01);

		// Update audio.
		if (m_audioServer)
		{
			T_PROFILER_SCOPE(L"Application update - Audio server");
			m_audioServer->update((float)m_updateInfo.m_frameDeltaTime, m_renderViewActive);
		}

		// Update rumble.
		if (m_inputServer)
		{
			T_PROFILER_SCOPE(L"Application update - Rumble");
			m_inputServer->updateRumble((float)m_updateInfo.m_frameDeltaTime, m_updateControl.m_pause);
		}

		// Update active state; fixed time step if physics manager is available.
		const physics::PhysicsManager* physicsManager = m_physicsServer ? m_physicsServer->getPhysicsManager() : nullptr;
		if (physicsManager && !m_updateControl.m_pause)
		{
			const double dT = m_updateControl.m_timeScale / m_updateControl.m_simulationFrequency;
			const double dFT = m_updateInfo.m_frameDeltaTime * m_updateControl.m_timeScale;

			m_updateInfo.m_simulationDeltaTime = dT;
			m_updateInfo.m_simulationFrequency = 1.0 / dT;

			// Calculate number of required updates in order to
			// keep game in sync with render time.
			const double simulationEndTime = (m_updateInfo.m_stateTime + dFT);
			const int32_t updateCountNoClamp = int32_t((simulationEndTime - m_updateInfo.m_simulationTime) / dT);
			updateCount = std::min(updateCountNoClamp, m_maxSimulationUpdates);

			// Execute fixed update(s).
			bool renderCollision = false;
			for (int32_t i = 0; i < updateCount; ++i)
			{
				// If we're doing multiple updates per frame then we're rendering bound; so in order
				// to keep input updating periodically we need to make sure we wait a bit, as long
				// as we don't collide with end-of-rendering.
				if (m_threadRender && i > 0 && !renderCollision)
				{
					// Recalculate interval for each sub-step as some updates might spike.
					const double excessTime = std::max(m_renderCpuDurations[1] - m_buildDuration - m_updateDuration * updateCount, 0.0);
					updateInterval = std::min(excessTime / updateCount, 0.03);

					// Need some wait margin as events, especially on Windows, have very low accuracy.
					if (m_signalRenderFinish.wait(int32_t(updateInterval * 1000.0)))
					{
						// Rendering of last frame has already finished; do not wait further intervals during this
						// update phase, just continue as fast as possible.
						renderCollision = true;
						++m_renderCollisions;
					}
				}

				// Update input.
				const double inputTimeStart = m_timer.getElapsedTime();
				if (m_inputServer)
				{
					T_PROFILER_SCOPE(L"Application update - Input server");
					m_inputServer->update((float)m_updateInfo.m_simulationDeltaTime, inputEnabled);
				}
				const double inputTimeEnd = m_timer.getElapsedTime();
				inputDuration += inputTimeEnd - inputTimeStart;

				// Update current state for each simulation tick.
				const double updateTimeStart = m_timer.getElapsedTime();
				IState::UpdateResult updateResult;
				{
					T_PROFILER_SCOPE(L"Application update - State");
					updateResult = currentState->update(m_stateManager, m_updateInfo);
				}
				const double updateTimeEnd = m_timer.getElapsedTime();
				updateDuration += updateTimeEnd - updateTimeStart;

				// Update physics.
				const double physicsTimeStart = m_timer.getElapsedTime();
				{
					T_PROFILER_SCOPE(L"Application update - Physics server");
					m_physicsServer->update((float)m_updateInfo.m_simulationDeltaTime);
				}
				const double physicsTimeEnd = m_timer.getElapsedTime();
				physicsDuration += physicsTimeEnd - physicsTimeStart;

				// Post physics update state.
				{
					T_PROFILER_SCOPE(L"Application post update - State");
					currentState->postUpdate(m_stateManager, m_updateInfo);
				}

				m_updateDuration = physicsTimeEnd - physicsTimeStart + inputTimeEnd - inputTimeStart + updateTimeEnd - updateTimeStart;
				m_updateInfo.m_simulationTime += dT;

				m_updateInfo.m_totalTime += (dFT / updateCount);
				m_updateInfo.m_stateTime += (dFT / updateCount);

				if (updateResult == IState::UrExit || updateResult == IState::UrFailed)
				{
					// Ensure render thread is finished before we leave.
					if (m_threadRender)
						m_signalRenderFinish.wait(1000);
					return false;
				}

				// Abort simulation loop if state has been changed.
				if (m_stateManager->getNext())
					break;
			}

			if (updateCount == 0)
			{
				m_updateInfo.m_totalTime += dFT;
				m_updateInfo.m_stateTime += dFT;
			}

			// Cannot allow time to drift even if number of updates are clamped,
			// this is quite a bad situation.
			m_updateInfo.m_simulationTime += dT * (updateCountNoClamp - updateCount);

			// Calculate interval value; interval is only required if simulation runs slower than render framerate.
			if (dT > dFT + 0.01)
				m_updateInfo.m_interval = (float)((m_updateInfo.m_stateTime - m_updateInfo.m_simulationTime) / m_updateInfo.m_simulationDeltaTime);
			else
				m_updateInfo.m_interval = 1.0f;
		}
		else
		{
			// Update input.
			if (m_inputServer)
			{
				T_PROFILER_SCOPE(L"Application update - Input server");
				m_inputServer->update((float)m_updateInfo.m_frameDeltaTime, inputEnabled);
			}

			// Update in same rate as rendering.
			m_updateInfo.m_simulationDeltaTime = m_updateInfo.m_frameDeltaTime * m_updateControl.m_timeScale;
			m_updateInfo.m_simulationFrequency = 1.0 / m_updateInfo.m_frameDeltaTime;

			const double updateTimeStart = m_timer.getElapsedTime();
			IState::UpdateResult updateResult;
			{
				T_PROFILER_SCOPE(L"Application update - State");
				updateResult = currentState->update(m_stateManager, m_updateInfo);
			}
			const double updateTimeEnd = m_timer.getElapsedTime();
			updateDuration += updateTimeEnd - updateTimeStart;
			updateCount++;

			// Issue post update.
			if (updateResult == IState::UrOk)
			{
				T_PROFILER_SCOPE(L"Application post update - State");
				updateResult = currentState->postUpdate(m_stateManager, m_updateInfo);
			}

			m_updateInfo.m_simulationTime += m_updateInfo.m_simulationDeltaTime;
			m_updateInfo.m_totalTime += m_updateInfo.m_simulationDeltaTime;
			m_updateInfo.m_stateTime += m_updateInfo.m_simulationDeltaTime;
			m_updateInfo.m_interval = 1.0f;

			if (updateResult == IState::UrExit || updateResult == IState::UrFailed)
			{
				// Ensure render thread is finished before we leave.
				if (m_threadRender)
					m_signalRenderFinish.wait(1000);
				return false;
			}

			// Yield a lot of CPU if game is paused.
			if (m_updateControl.m_pause && m_pauseYield)
			{
				Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
				if (currentThread)
					currentThread->sleep(50);
			}
		}

		// Leave if state has been changed; no need to render current state.
		if (m_stateManager->getNext())
			return true;

		// Build frame.
		const double buildTimeStart = m_timer.getElapsedTime();
		IState::BuildResult buildResult;
		{
			T_PROFILER_SCOPE(L"Application build - State");
			buildResult = currentState->build(m_frameBuild, m_updateInfo);
		}
		const double buildTimeEnd = m_timer.getElapsedTime();
		m_buildDuration = buildTimeEnd - buildTimeStart;

		// Update scripting language runtime.
		const double gcTimeStart = m_timer.getElapsedTime();
		if (m_scriptServer)
		{
			T_PROFILER_SCOPE(L"Application script GC");
			m_scriptServer->cleanup(false);
		}
		const double gcTimeEnd = m_timer.getElapsedTime();
		gcDuration = gcTimeEnd - gcTimeStart;

		if (buildResult == IState::BrOk || buildResult == IState::BrNothing)
		{
			if (m_threadRender)
			{
				T_PROFILER_SCOPE(L"Application sync render");

				// Synchronize with render thread and issue another rendering.
				const bool renderFinished = m_signalRenderFinish.wait(1000);
				if (renderFinished)
				{
					m_signalRenderFinish.reset();

					{
						T_ANONYMOUS_VAR(Acquire< TicketLock >)(m_lockRender);
						T_ASSERT(m_stateRender == nullptr);

						m_frameRender = m_frameBuild;
						m_stateRender = currentState;
						m_updateInfoRender = m_updateInfo;
						m_frameBuild = (m_frameBuild + 1) % m_renderServer->getThreadFrameQueueCount();
					}

					m_signalRenderBegin.set();
				}
			}
			else
			{
				T_PROFILER_SCOPE(L"Application render");
				const double renderBegin = m_timer.getElapsedTime();

				// Single threaded rendering; perform rendering here.
				render::IRenderView* renderView = m_renderServer->getRenderView();
				if (renderView && !renderView->isMinimized())
				{
					T_PROFILER_BEGIN(L"Application render beginFrame");
					const bool frameBegun = renderView->beginFrame();
					T_PROFILER_END();

					if (frameBegun)
					{
						bool submittedQuery = false;
						if (m_renderGpuDurationQuery < 0)
						{
							m_renderGpuDurationQuery = renderView->beginTimeQuery();
							submittedQuery = true;
						}

						if (currentState)
							currentState->render(m_frameRender, m_updateInfoRender);

						if (submittedQuery)
							renderView->endTimeQuery(m_renderGpuDurationQuery);

						T_PROFILER_BEGIN(L"Application render endFrame");
						renderView->endFrame();
						T_PROFILER_END();

						const double renderEnd = m_timer.getElapsedTime();
						m_renderCpuDurations[0] = renderEnd - renderBegin;
					}
					
					T_PROFILER_BEGIN(L"Application render present");
					renderView->present();
					T_PROFILER_END();

					if (m_renderGpuDurationQuery >= 0)
					{
						double start, end;
						if (renderView->getTimeQuery(m_renderGpuDurationQuery, false, start, end))
						{
							m_renderGpuDuration = end - start;
							m_renderGpuDurationQuery = -1;
						}
					}

					T_PROFILER_BEGIN(L"Application render stats");
					renderView->getStatistics(m_renderViewStats);
					T_PROFILER_END();
				}
				else
				{
					// Yield main thread.
					m_threadRender->sleep(10);
				}

				const double renderEnd = m_timer.getElapsedTime();
				m_renderCpuDurations[1] = renderEnd - renderBegin;

				m_renderServer->setFrameRate(int32_t(1.0 / m_renderCpuDurations[1]));
			}
		}

		// In case nothing has been built we yield main thread in favor of other
		// threads created by the application.
		if (buildResult == IState::BrNothing)
		{
			Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
			if (currentThread)
				currentThread->yield();
		}

		m_updateInfo.m_frame++;

		// Receive remote events from editor.
		if (m_targetManagerConnection && m_targetManagerConnection->connected())
		{
			T_PROFILER_SCOPE(L"Application remote events");
			Ref< IRemoteEvent > remoteEvent;
			if (
				m_targetManagerConnection->getTransport()->recv< IRemoteEvent >(0, remoteEvent) == net::BidirectionalObjectTransport::Result::Success &&
				remoteEvent != nullptr
			)
			{
				currentState->take(remoteEvent);
			}
		}

#if T_MEASURE_PERFORMANCE
		// Publish performance to target manager.
		if (m_targetManagerConnection && m_targetManagerConnection->connected())
		{
			T_PROFILER_SCOPE(L"Application telemetry");

			// Runtime
			{
				TpsRuntime tp;
				tp.fps = (float)(1.0 / m_updateInfo.m_frameDeltaTime);
				if (updateCount > 0)
				{
					tp.update = (float)(updateDuration / updateCount);
					tp.physics = (float)(physicsDuration / updateCount);
					tp.input = (float)(inputDuration / updateCount);
				}		
				tp.build = (float)(buildTimeEnd - buildTimeStart);
				tp.renderCPU = (float)m_renderCpuDurations[0];
				tp.renderGPU = (float)m_renderGpuDuration;
				tp.garbageCollect = (float)gcDuration;
				tp.steps = updateCount;
				tp.simulationInterval = (float)updateInterval;
				tp.renderInterval = m_updateInfo.getInterval();
				tp.collisions = m_renderCollisions;
				m_targetPerformance.publish(m_targetManagerConnection->getTransport(), tp);
			}

			// Memory
			{
				TpsMemory tp;
				tp.memInUse = (uint64_t)Alloc::allocated();
				tp.heapObjects = Object::getHeapObjectCount();
				if (m_scriptServer)
				{
					script::ScriptStatistics ss;
					m_scriptServer->getScriptManager()->getStatistics(ss);
					tp.memInUseScript = ss.memoryUsage;
				}
				m_targetPerformance.publish(m_targetManagerConnection->getTransport(), tp);
			}

			// Render
			{
				TpsRender tp;
				m_renderServer->getRenderSystem()->getStatistics(tp.renderSystemStats);
				tp.renderViewStats = m_renderViewStats;
				m_targetPerformance.publish(m_targetManagerConnection->getTransport(), tp);
			}

			// Resource
			{
				TpsResource tp;
				resource::ResourceManagerStatistics rms;
				m_resourceServer->getResourceManager()->getStatistics(rms);
				tp.residentResourcesCount = rms.residentCount;
				tp.exclusiveResourcesCount = rms.exclusiveCount;
				m_targetPerformance.publish(m_targetManagerConnection->getTransport(), tp);
			}

			// Physics
			{
				TpsPhysics tp;
				if (m_physicsServer)
				{
					physics::PhysicsStatistics ps;
					m_physicsServer->getPhysicsManager()->getStatistics(ps);
					tp.bodyCount = ps.bodyCount;
					tp.activeBodyCount = ps.activeCount;
					tp.manifoldCount = ps.manifoldCount;
					tp.queryCount = ps.queryCount;
				}
				m_targetPerformance.publish(m_targetManagerConnection->getTransport(), tp);
			}

			// Audio
			{
				TpsAudio tp;
				if (m_audioServer)
					tp.activeSoundChannels = m_audioServer->getActiveSoundChannels();
				m_targetPerformance.publish(m_targetManagerConnection->getTransport(), tp);
			}
		}
#endif
	}

	return true;
}

void Application::suspend()
{
	T_ANONYMOUS_VAR(Acquire< TicketLock >)(m_lockRender);
	if (m_stateManager->getCurrent() != nullptr)
	{
		const ActiveEvent activeEvent(false);
		m_stateManager->getCurrent()->take(&activeEvent);
	}
}

void Application::resume()
{
	T_ANONYMOUS_VAR(Acquire< TicketLock >)(m_lockRender);
	if (m_stateManager->getCurrent() != nullptr)
	{
		const ActiveEvent activeEvent(true);
		m_stateManager->getCurrent()->take(&activeEvent);
	}
}

IEnvironment* Application::getEnvironment()
{
	return m_environment;
}

IStateManager* Application::getStateManager()
{
	return m_stateManager;
}

void Application::pollDatabase()
{
	T_PROFILER_SCOPE(L"Application poll database");
	AlignedVector< Guid > eventIds;
	Ref< const db::IEvent > event;
	bool remote;

	const Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
	while (!currentThread->stopped() && m_database->getEvent(event, remote))
	{
		if (auto committed = dynamic_type_cast< const db::EvtInstanceCommitted* >(event))
			eventIds.push_back(committed->getInstanceGuid());
	}

	if (!currentThread->stopped() && !eventIds.empty())
	{
		T_ANONYMOUS_VAR(Acquire< TicketLock >)(m_lockUpdate);
		T_ANONYMOUS_VAR(Acquire< TicketLock >)(m_lockRender);

		if (auto currentState = m_stateManager->getCurrent())
		{
			const HotReloadEvent hotReloadEvent;
			currentState->take(&hotReloadEvent);
		}

		Ref< resource::IResourceManager > resourceManager = m_resourceServer->getResourceManager();
		if (resourceManager)
		{
			std::reverse(eventIds.begin(), eventIds.end());
			for (const auto& eventId : eventIds)
			{
				log::info << L"Reloading resource \"" << eventId.format() << L"\"..." << Endl;
				resourceManager->reload(eventId, false);
			}
		}

		m_environment->reconfigure();
	}
}

void Application::threadDatabase()
{
	while (!m_threadDatabase->stopped())
	{
		for (uint32_t i = 0; i < c_databasePollInterval && !m_threadDatabase->stopped(); ++i)
			m_threadDatabase->sleep(100);
		pollDatabase();
	}
}

void Application::threadRender()
{
	// We're ready to begin rendering.
	m_signalRenderFinish.set();

	while (!m_threadRender->stopped())
	{
		// Wait until we have a frame to render.
		if (!m_signalRenderBegin.wait(100))
			continue;

		{
			m_signalRenderBegin.reset();

			// Render frame.
			{
				T_ANONYMOUS_VAR(Acquire< TicketLock >)(m_lockRender);

				const double renderBegin = m_timer.getElapsedTime();

				render::IRenderView* renderView = m_renderServer->getRenderView();
				if (renderView && !renderView->isMinimized())
				{
					T_PROFILER_BEGIN(L"Application render");

					T_PROFILER_BEGIN(L"Application render beginFrame");
					const bool frameBegun = renderView->beginFrame();
					T_PROFILER_END();

					if (frameBegun)
					{
						bool submittedQuery = false;
						if (m_renderGpuDurationQuery < 0)
						{
							m_renderGpuDurationQuery = renderView->beginTimeQuery();
							submittedQuery = true;
						}

						if (m_stateRender)
							m_stateRender->render(m_frameRender, m_updateInfoRender);

						if (submittedQuery)
							renderView->endTimeQuery(m_renderGpuDurationQuery);

						T_PROFILER_BEGIN(L"Application render endFrame");
						renderView->endFrame();
						T_PROFILER_END();

						const double renderEnd = m_timer.getElapsedTime();
						m_renderCpuDurations[0] = renderEnd - renderBegin;

						T_PROFILER_BEGIN(L"Application render present");
						renderView->present();
						T_PROFILER_END();
					}

					if (m_renderGpuDurationQuery >= 0)
					{
						double start, end;
						if (renderView->getTimeQuery(m_renderGpuDurationQuery, false, start, end))
						{
							m_renderGpuDuration = end - start;
							m_renderGpuDurationQuery = -1;
						}
					}

					T_PROFILER_BEGIN(L"Application render stats");
					renderView->getStatistics(m_renderViewStats);
					T_PROFILER_END();

					T_PROFILER_END();
				}
				else
				{
					// Yield render thread.
					m_threadRender->sleep(100);
				}

				const double renderEnd = m_timer.getElapsedTime();
				m_renderCpuDurations[1] = renderEnd - renderBegin;

				m_renderServer->setFrameRate(int32_t(1.0 / m_renderCpuDurations[1]));
				m_stateRender = nullptr;
			}

			// Frame finished.
			m_signalRenderFinish.set();
		}
	}
}

}

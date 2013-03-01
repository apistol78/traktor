#include "Amalgam/IRuntimePlugin.h"
#include "Amalgam/IState.h"
#include "Amalgam/Events/ActiveEvent.h"
#include "Amalgam/Events/ReconfigureEvent.h"
#include "Amalgam/Impl/Application.h"
#include "Amalgam/Impl/AudioServer.h"
#include "Amalgam/Impl/Environment.h"
#include "Amalgam/Impl/StateManager.h"
#include "Amalgam/Impl/InputServer.h"
#include "Amalgam/Impl/OnlineServer.h"
#include "Amalgam/Impl/PhysicsServer.h"
#include "Amalgam/Impl/RenderServerDefault.h"
#include "Amalgam/Impl/RenderServerEmbedded.h"
#include "Amalgam/Impl/ResourceServer.h"
#include "Amalgam/Impl/ScriptServer.h"
#include "Amalgam/Impl/TargetManagerConnection.h"
#include "Amalgam/Impl/TargetPerformance.h"
#include "Amalgam/Impl/WorldServer.h"
#include "Core/Platform.h"
#include "Core/Library/Library.h"
#include "Core/Log/Log.h"
#include "Core/Math/Float.h"
#include "Core/Math/MathUtils.h"
#include "Core/Memory/Alloc.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
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
#include "Database/Database.h"
#include "Database/Events/EvtInstanceCommitted.h"
#include "Online/ISessionManager.h"
#include "Net/BidirectionalObjectTransport.h"
#include "Physics/PhysicsManager.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

const int32_t c_databasePollInterval = 5;
const uint32_t c_simulationFrequency = 60;
const float c_simulationDeltaTime = 1.0f / c_simulationFrequency;
const float c_maxDeltaTime = 0.5f;
const float c_minDeltaTime = 0.0f;
const float c_deltaTimeFilterCoeff = 0.99f;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.Application", Application, IApplication)

Application::Application()
:	m_threadDatabase(0)
,	m_threadRender(0)
,	m_maxSimulationUpdates(1)
,	m_renderViewActive(true)
,	m_backgroundColor(0.0f, 0.0f, 0.0f, 0.0f)
,	m_updateDuration(0.0f)
,	m_buildDuration(0.0f)
,	m_renderDuration(0.0f)
,	m_renderCollisions(0)
,	m_frameBuild(0)
,	m_frameRender(0)
,	m_stateRender(0)
#if T_MEASURE_PERFORMANCE
,	m_fps(0.0f)
#endif
{
}

bool Application::create(
	const PropertyGroup* defaultSettings,
	PropertyGroup* settings,
	void* nativeWindowHandle
)
{
	// Establish target manager connection is launched from the Editor.
	std::wstring targetManagerHost = settings->getProperty< PropertyString >(L"Amalgam.TargetManager/Host");
	int32_t targetManagerPort = settings->getProperty< PropertyInteger >(L"Amalgam.TargetManager/Port");
	Guid targetManagerId = Guid(settings->getProperty< PropertyString >(L"Amalgam.TargetManager/Id"));
	if (!targetManagerHost.empty() && targetManagerPort && targetManagerId.isValid())
	{
		m_targetManagerConnection = new TargetManagerConnection();
		if (!m_targetManagerConnection->connect(targetManagerHost, targetManagerPort, targetManagerId))
		{
			log::warning << L"Unable to connect to target manager at \"" << targetManagerHost << L"\"; unable to debug" << Endl;
			m_targetManagerConnection = 0;
		}
	}

	// Load dependent modules.
#if !defined(T_STATIC)
	std::set< std::wstring > modules = settings->getProperty< PropertyStringSet >(L"Amalgam.Modules");
	for (std::set< std::wstring >::const_iterator i = modules.begin(); i != modules.end(); ++i)
	{
		Ref< Library > library = new Library();
		if (!library->open(*i))
		{
			log::error << L"Application failed; unable to load module \"" << *i << L"\"" << Endl;
			return false;
		}
		m_libraries.push_back(library);
	}
#endif

	// Sub step configuration.
	m_maxSimulationUpdates = settings->getProperty< PropertyInteger >(L"Amalgam.MaxSimulationUpdates", 4);
	m_maxSimulationUpdates = max(m_maxSimulationUpdates, 1);

	m_stateManager = new StateManager();

	// Database
	T_DEBUG(L"Creating database...");
	m_database = new db::Database ();
	std::wstring connectionString = settings->getProperty< PropertyString >(L"Amalgam.Database");
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
		if (!m_onlineServer->create(settings, m_database))
			return false;
	}

	// Render
	T_DEBUG(L"Creating render server...");
	if (nativeWindowHandle)
	{
		Ref< RenderServerEmbedded > renderServer = new RenderServerEmbedded();
		if (!renderServer->create(settings, nativeWindowHandle))
			return false;
		m_renderServer = renderServer;
	}
	else
	{
		Ref< RenderServerDefault > renderServer = new RenderServerDefault();
		if (!renderServer->create(settings))
			return false;
		m_renderServer = renderServer;
	}

	// Resource
	T_DEBUG(L"Creating resource server...");
	m_resourceServer = new ResourceServer();
	if (!m_resourceServer->create(settings))
		return false;

	// Input
	T_DEBUG(L"Creating input server...");
	m_inputServer = new InputServer();
	if (!m_inputServer->create(
		defaultSettings,
		settings,
		m_database,
		m_renderServer->getRenderView()->getSystemWindow()
	))
		return false;

	// Physics
	if (settings->getProperty(L"Physics.Type"))
	{
		T_DEBUG(L"Creating physics server...");
		m_physicsServer = new PhysicsServer();
		if (!m_physicsServer->create(settings, c_simulationDeltaTime))
			return false;
	}

	// Script
	if (settings->getProperty(L"Script.Type"))
	{
		T_DEBUG(L"Creating script server...");
		m_scriptServer = new ScriptServer();
		if (settings->getProperty< PropertyBoolean >(L"Script.AttachDebugger", false) && m_targetManagerConnection)
		{
			if (!m_scriptServer->create(settings, true, m_targetManagerConnection->getTransport()))
				return false;
		}
		else
		{
			if (!m_scriptServer->create(settings, false, 0))
				return false;
		}
	}

	// World
	T_DEBUG(L"Creating world server...");
	m_worldServer = new WorldServer();
	if (!m_worldServer->create(settings, m_renderServer, m_resourceServer))
		return false;

	// Audio
	T_DEBUG(L"Creating audio server...");
	if (settings->getProperty(L"Audio.Type"))
	{
		m_audioServer = new AudioServer();
		if (!m_audioServer->create(settings))
			return false;
	}

	// Environment
	T_DEBUG(L"Creating environment...");
	m_environment = new Environment(
		settings,
		m_database,
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
	if (m_physicsServer)
		m_physicsServer->createEntityFactories(m_environment);
	m_worldServer->createEntityFactories(m_environment);

	// Create plugins.
	T_DEBUG(L"Creating plugins...");

	std::vector< const TypeInfo* > pluginTypes;
	type_of< IRuntimePlugin >().findAllOf(pluginTypes, false);

	for (std::vector< const TypeInfo* >::const_iterator i = pluginTypes.begin(); i != pluginTypes.end(); ++i)
	{
		T_ASSERT (*i);

		Ref< IRuntimePlugin > plugin = dynamic_type_cast< IRuntimePlugin* >((*i)->createInstance());
		if (!plugin)
		{
			log::error << L"Application failed; unable to instantiate plugin \"" << (*i)->getName() << L"\"" << Endl;
			return false;
		}

		m_plugins.push_back(plugin);
	}

	for (uint32_t i = 0; i < m_plugins.size(); )
	{
		TypeInfoSet dependencies;
		m_plugins[i]->getDependencies(dependencies);

		bool satisfied = true;

		for (TypeInfoSet::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
		{
			satisfied = false;
			for (uint32_t k = 0; k < i; ++k)
			{
				if (*j == &type_of(m_plugins[k]))
				{
					satisfied = true;
					break;
				}
			}
			if (!satisfied)
				break;
		}

		if (!satisfied)
		{
			if (i < m_plugins.size() - 1)
			{
				m_plugins.push_back(m_plugins[i]);
				m_plugins.erase(m_plugins.begin() + i);
			}
			else
			{
				log::error << L"Application failed; unable to resolve plugin \"" << type_name(m_plugins[i]) << L"\" dependencies" << Endl;
				return false;
			}
		}
		else
		{
			if (!m_plugins[i]->startup(m_environment))
			{
				log::error << L"Application failed; unable to start plugin \"" << type_name(m_plugins[i]) << L"\"" << Endl;
				return false;
			}
			++i;
		}
	}

	// Database monitoring thread.
	if (settings->getProperty< PropertyBoolean >(L"Amalgam.DatabaseThread", false))
	{
		T_DEBUG(L"Creating database monitoring thread...");
		m_threadDatabase = ThreadManager::getInstance().create(makeFunctor(this, &Application::threadDatabase), L"Database events");
		if (m_threadDatabase)
			m_threadDatabase->start(Thread::Highest);
	}

	// Initial, startup, state.
	T_DEBUG(L"Creating initial state...");

	Ref< IState > state;
	for (RefArray< IRuntimePlugin >::const_iterator i = m_plugins.begin(); i != m_plugins.end(); ++i)
	{
		state = (*i)->createInitialState(m_environment);
		if (state)
			break;
	}

	if (!state)
	{
		log::error << L"Application failed; unable to create initial state" << Endl;
		return false;
	}

	log::info << L"Application started successfully; enter initial state..." << Endl;

	m_timer.start();
	m_stateManager->enter(state);

	log::info << L"Initial state ready; enter main loop..." << Endl;

	// Create render thread if enabled and we're running on a multi core system.
	if (
		OS::getInstance().getCPUCoreCount() >= 2 &&
		settings->getProperty< PropertyBoolean >(L"Amalgam.RenderThread", true)
	)
	{
		m_threadRender = ThreadManager::getInstance().create(makeFunctor(this, &Application::threadRender), L"Render");
		if (m_threadRender)
		{
			m_threadRender->start(Thread::Above);
			if (m_signalRenderFinish.wait(1000))
				log::info << L"Render thread started successfully" << Endl;
			else
				log::warning << L"Unable to synchronize render thread" << Endl;
		}
	}
	else
		log::info << L"Using single threaded rendering" << Endl;

	m_settings = settings;
	return true;
}

void Application::destroy()
{
	for (RefArray< IRuntimePlugin >::iterator i = m_plugins.begin(); i != m_plugins.end(); ++i)
		(*i)->shutdown(m_environment);

	m_plugins.resize(0);

	if (m_threadRender)
	{
		m_threadRender->stop();
		ThreadManager::getInstance().destroy(m_threadRender);
		m_threadRender = 0;
	}

	if (m_threadDatabase)
	{
		m_threadDatabase->stop();
		ThreadManager::getInstance().destroy(m_threadDatabase);
		m_threadDatabase = 0;
	}

	JobManager::getInstance().stop();

	safeDestroy(m_stateManager);

	safeDestroy(m_resourceServer);
	safeDestroy(m_worldServer);
	safeDestroy(m_scriptServer);
	safeDestroy(m_physicsServer);
	safeDestroy(m_onlineServer);
	safeDestroy(m_inputServer);
	safeDestroy(m_audioServer);
	safeDestroy(m_renderServer);

	m_environment = 0;

	if (m_database)
	{
		m_database->close();
		m_database = 0;
	}

	for (RefArray< Library >::iterator i = m_libraries.begin(); i != m_libraries.end(); ++i)
		(*i)->detach();
}

bool Application::update()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lockUpdate);
	Ref< IState > currentState;

	// Update target manager connection.
	if (m_targetManagerConnection)
	{
		if (!m_targetManagerConnection->update())
			return false;
	}

	// Update render server.
	RenderServer::UpdateResult updateResult = m_renderServer->update(m_settings);
	if (updateResult == RenderServer::UrTerminate)
		return false;

	// Perform reconfiguration if required.
	if (updateResult == RenderServer::UrReconfigure || m_environment->shouldReconfigure())
	{
		// Synchronize rendering thread first as renderer might be reconfigured.
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lockRender);

		// Execute configuration on all servers.
		int32_t result = m_environment->executeReconfigure();
		if (result == CrFailed)
			return false;

		// Emit action in current state as we've successfully reconfigured servers.
		if ((currentState = m_stateManager->getCurrent()) != 0)
		{
			ReconfigureEvent configureEvent(result);
			currentState->take(&configureEvent);
		}
	}

	// Update online session manager.
	if (m_onlineServer)
	{
		online::ISessionManager* sessionManager = m_onlineServer->getSessionManager();
		if (sessionManager)
			sessionManager->update();
	}

	// Handle state transitions.
	if (m_stateManager->beginTransition())
	{
		// Transition begun; need to synchronize rendering thread as
		// it require current state.

		if (m_threadRender && !m_signalRenderFinish.wait(1000))
		{
			log::error << L"Unable to synchronize render thread; render thread seems to be stuck" << Endl;
			return false;
		}

		// Ensure state transition is safe.
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lockRender);
			T_FATAL_ASSERT (m_stateRender == 0);

			// Leave current state.
			T_ASSERT_M (m_stateManager->getCurrent() == 0 || m_stateManager->getCurrent()->getReferenceCount() == 1, L"Current state must have one reference only");
			log::info << L"Leaving state \"" << type_name(m_stateManager->getCurrent()) << L"\"..." << Endl;
			m_stateManager->leaveCurrent();

			// Cleanup resources used by former state.
			log::info << L"Cleaning resident resources..." << Endl;
			m_resourceServer->performCleanup();

			// Reset time data.
			m_updateInfo.m_frameDeltaTime = c_simulationDeltaTime;
			m_updateInfo.m_simulationTime = 0.0f;
			m_updateInfo.m_stateTime = 0.0f;

			// Enter new state.
			T_ASSERT_M (m_stateManager->getNext() && m_stateManager->getNext()->getReferenceCount() == 1, L"Next state must exist and have one reference only");
			log::info << L"Enter state \"" << type_name(m_stateManager->getNext()) << L"\"..." << Endl;
			m_stateManager->enterNext();

			// Assume state's active from start.
			m_renderViewActive = true;
			log::info << L"State transition complete" << Endl;
		}
	}

	// Update scripting language runtime.
	if (m_scriptServer)
		m_scriptServer->update();

	if ((currentState = m_stateManager->getCurrent()) != 0)
	{
		// Check render active state; notify application when changes.
		bool renderViewActive = m_renderServer->getRenderView()->isActive();
		if (renderViewActive != m_renderViewActive)
		{
			ActiveEvent activeEvent(renderViewActive);
			currentState->take(&activeEvent);
			m_renderViewActive = renderViewActive;
		}

		// Determine if input should be enabled.
		bool inputEnabled = m_renderViewActive;
		if (m_onlineServer)
			inputEnabled &= !m_onlineServer->getSessionManager()->requireUserAttention();

		// Measure delta time; filter frame delta time to prevent unneccessary jitter.
		float deltaTime = float(m_timer.getDeltaTime());
		deltaTime = min(deltaTime, c_maxDeltaTime);
		deltaTime = max(deltaTime, c_minDeltaTime);
		m_updateInfo.m_frameDeltaTime = deltaTime * c_deltaTimeFilterCoeff + m_updateInfo.m_frameDeltaTime * (1.0f - c_deltaTimeFilterCoeff);

		// Update audio.
		if (m_audioServer)
			m_audioServer->update(m_updateInfo.m_frameDeltaTime, m_renderViewActive);

		// Update rumble.
		if (m_inputServer)
			m_inputServer->updateRumble(m_updateInfo.m_frameDeltaTime, m_updateControl.m_pause);

		// Update active state; fixed time step if physics manager is available.
		double updateDuration = 0.0;
		double physicsDuration = 0.0;
		double inputDuration = 0.0;
		float updateInterval = 0.0f;
		int32_t updateCount = 0;

		physics::PhysicsManager* physicsManager = m_physicsServer ? m_physicsServer->getPhysicsManager() : 0;
		if (physicsManager && !m_updateControl.m_pause)
		{
			m_updateInfo.m_simulationDeltaTime = c_simulationDeltaTime;
			m_updateInfo.m_simulationFrequency = c_simulationFrequency;

			float simulationEndTime = m_updateInfo.m_stateTime;
			updateCount = std::min(int32_t((simulationEndTime - m_updateInfo.m_simulationTime) / c_simulationDeltaTime), m_maxSimulationUpdates);

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
					float excessTime = std::max(m_renderDuration - m_buildDuration - m_updateDuration * updateCount, 0.0f);
					updateInterval = std::min(excessTime / updateCount, 0.03f);

					// Need some wait margin as events, especially on Windows, have very low accuracy.
					if (m_signalRenderFinish.wait(int32_t(updateInterval * 1000.0f)))
					{
						// Rendering of last frame has already finished; do not wait further intervals during this
						// update phase, just continue as fast as possible.
						renderCollision = true;
						++m_renderCollisions;
					}
				}

				// Update input.
				double inputTimeStart = m_timer.getElapsedTime();
				if (m_inputServer)
					m_inputServer->update(m_updateInfo.m_simulationDeltaTime, inputEnabled);
				double inputTimeEnd = m_timer.getElapsedTime();
				inputDuration += inputTimeEnd - inputTimeStart;

				// Update current state for each simulation tick.
				double updateTimeStart = m_timer.getElapsedTime();
				IState::UpdateResult result = currentState->update(m_stateManager, m_updateControl, m_updateInfo);
				double updateTimeEnd = m_timer.getElapsedTime();
				updateDuration += updateTimeEnd - updateTimeStart;

				// Update physics.
				double physicsTimeStart = m_timer.getElapsedTime();
				m_physicsServer->update();
				double physicsTimeEnd = m_timer.getElapsedTime();
				physicsDuration += physicsTimeEnd - physicsTimeStart;

				m_updateDuration = float(physicsTimeEnd - physicsTimeStart + inputTimeEnd - inputTimeStart + updateTimeEnd - updateTimeStart);

				m_updateInfo.m_simulationTime += c_simulationDeltaTime;
				m_updateInfo.m_totalTime += c_simulationDeltaTime;
				m_updateInfo.m_stateTime += c_simulationDeltaTime;

				if (result == IState::UrExit || result == IState::UrFailed)
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

			m_updateInfo.m_totalTime += m_updateInfo.m_frameDeltaTime - updateCount * c_simulationDeltaTime;
			m_updateInfo.m_stateTime += m_updateInfo.m_frameDeltaTime - updateCount * c_simulationDeltaTime;
		}
		else
		{
			// Update input.
			if (m_inputServer)
				m_inputServer->update(m_updateInfo.m_frameDeltaTime, inputEnabled);

			// No physics; update in same rate as rendering.
			m_updateInfo.m_simulationTime = m_updateInfo.m_stateTime;
			m_updateInfo.m_simulationDeltaTime = m_updateInfo.m_frameDeltaTime;
			m_updateInfo.m_simulationFrequency = uint32_t(1.0f / m_updateInfo.m_frameDeltaTime);

			double updateTimeStart = m_timer.getElapsedTime();
			IState::UpdateResult updateResult = currentState->update(m_stateManager, m_updateControl, m_updateInfo);
			double updateTimeEnd = m_timer.getElapsedTime();
			updateDuration += updateTimeEnd - updateTimeStart;
			updateCount++;

			m_updateInfo.m_totalTime += m_updateInfo.m_frameDeltaTime;
			m_updateInfo.m_stateTime += m_updateInfo.m_frameDeltaTime;

			if (updateResult == IState::UrExit || updateResult == IState::UrFailed)
			{
				// Ensure render thread is finished before we leave.
				if (m_threadRender)
					m_signalRenderFinish.wait(1000);
				return false;
			}

			// Yield a lot of cpu if game is paused.
			if (m_updateControl.m_pause)
			{
				Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
				if (currentThread)
					currentThread->sleep(100);
			}
		}

		// Leave if state has been changed; no need to render current state.
		if (m_stateManager->getNext())
			return true;

		// Build frame.
		double buildTimeStart = m_timer.getElapsedTime();
		IState::BuildResult buildResult = currentState->build(m_frameBuild, m_updateInfo);
		double buildTimeEnd = m_timer.getElapsedTime();

		m_buildDuration = float(buildTimeEnd - buildTimeStart);

		if (buildResult == IState::BrOk || buildResult == IState::BrNothing)
		{
			if (m_threadRender)
			{
				// Synchronize with render thread and issue another rendering.
				bool renderFinished = m_signalRenderFinish.wait(1000);
				if (renderFinished)
				{
					m_signalRenderFinish.reset();

					{
						T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lockRender);
						T_ASSERT (m_stateRender == 0);

						m_frameRender = m_frameBuild;
						m_stateRender = currentState;
						m_updateInfoRender = m_updateInfo;
						m_frameBuild = (m_frameBuild + 1) % m_worldServer->getFrameCount();
					}

					m_signalRenderBegin.set();
				}
			}
			else
			{
				// Single threaded rendering; perform rendering here.
				render::IRenderView* renderView = m_renderServer->getRenderView();
				if (renderView)
				{
					if (!m_renderServer->getStereoscopic())
					{
						if (renderView->begin(render::EtCyclop))
						{
							if (currentState)
								currentState->render(m_frameBuild, render::EtCyclop, m_updateInfo);
							else
							{
								renderView->clear(
									render::CfColor | render::CfDepth | render::CfStencil,
									&m_backgroundColor,
									1.0f,
									0
								);
							}

							renderView->end();
							renderView->present();
						}
					}
					else
					{
						if (renderView->begin(render::EtLeft))
						{
							if (currentState)
								currentState->render(m_frameBuild, render::EtLeft, m_updateInfo);
							else
							{
								renderView->clear(
									render::CfColor | render::CfDepth | render::CfStencil,
									&m_backgroundColor,
									1.0f,
									0
								);
							}

							renderView->end();
						}
						if (renderView->begin(render::EtRight))
						{
							if (currentState)
								currentState->render(m_frameBuild, render::EtRight, m_updateInfo);
							else
							{
								renderView->clear(
									render::CfColor | render::CfDepth | render::CfStencil,
									&m_backgroundColor,
									1.0f,
									0
								);
							}

							renderView->end();
						}
						renderView->present();
					}
				}
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

#if T_MEASURE_PERFORMANCE
		// Calculate frame rate.
		m_fps = m_fps * 0.9f + (1.0f / m_updateInfo.m_frameDeltaTime) * 0.1f;

		// Publish performance to target manager.
		if (m_targetManagerConnection)
		{
			// First ensure target manager still connected.
			if (!m_targetManagerConnection->connected())
				return false;

			render::RenderViewStatistics rvs;
			m_renderServer->getRenderView()->getStatistics(rvs);

			resource::ResourceManagerStatistics rms;
			m_resourceServer->getResourceManager()->getStatistics(rms);

			TargetPerformance performance;
			performance.time = m_updateInfo.m_totalTime;
			performance.fps = m_fps;
			if (updateCount > 0)
			{
				performance.update = float(updateDuration / updateCount);
				performance.physics = float(physicsDuration / updateCount);
				performance.input = float(inputDuration / updateCount);
			}
			performance.steps = float(updateCount);
			performance.interval = updateInterval;
			performance.collisions = m_renderCollisions;
			performance.memInUse = Alloc::allocated();
			performance.heapObjects = Object::getHeapObjectCount();
			performance.build = float(buildTimeEnd - buildTimeStart);
#	if !defined(_PS3)
			performance.render = m_renderDuration;
#	else
			performance.render = float(rvs.duration);
#	endif
			performance.drawCalls = rvs.drawCalls;
			performance.primitiveCount = rvs.primitiveCount;
			performance.residentResourcesCount = rms.residentCount;
			performance.exclusiveResourcesCount = rms.exclusiveCount;

			if (m_physicsServer)
			{
				m_physicsServer->getPhysicsManager()->getBodyCount(
					performance.bodyCount,
					performance.activeBodyCount
				);
			}

			m_targetManagerConnection->getTransport()->send(&performance);
		}
#endif
	}

	return true;
}

Ref< IEnvironment > Application::getEnvironment()
{
	return m_environment;
}

Ref< IStateManager > Application::getStateManager()
{
	return m_stateManager;
}

void Application::threadDatabase()
{
	std::vector< Guid > eventIds;
	Ref< const db::IEvent > event;
	bool remote;

	while (!m_threadDatabase->stopped())
	{
		for (uint32_t i = 0; i < c_databasePollInterval && !m_threadDatabase->stopped(); ++i)
			m_threadDatabase->sleep(100);

		if (!m_database)
			continue;

		eventIds.resize(0);
		while (m_database->getEvent(event, remote))
		{
			if (const db::EvtInstanceCommitted* committed = dynamic_type_cast< const db::EvtInstanceCommitted* >(event))
				eventIds.push_back(committed->getInstanceGuid());
		}

		if (!eventIds.empty())
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lockUpdate);
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lockRender);

			resource::IResourceManager* resourceManager = m_resourceServer->getResourceManager();
			if (resourceManager)
			{
				for (std::vector< Guid >::iterator i = eventIds.begin(); i != eventIds.end(); ++i)
				{
					T_DEBUG(L"External database event; reloading resource \"" << i->format() << L"\"");
					resourceManager->reload(*i);
				}
			}
		}
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

		m_signalRenderBegin.reset();

		// Render frame.
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lockRender);

			double renderBegin = m_timer.getElapsedTime();

			render::IRenderView* renderView = m_renderServer->getRenderView();
			if (renderView)
			{
				if (!m_renderServer->getStereoscopic())
				{
					if (renderView->begin(render::EtCyclop))
					{
						if (m_stateRender)
							m_stateRender->render(m_frameRender, render::EtCyclop, m_updateInfoRender);
						else
						{
							renderView->clear(
								render::CfColor | render::CfDepth | render::CfStencil,
								&m_backgroundColor,
								1.0f,
								0
							);
						}

						renderView->end();
						renderView->present();
					}
				}
				else
				{
					if (renderView->begin(render::EtLeft))
					{
						if (m_stateRender)
							m_stateRender->render(m_frameRender, render::EtLeft, m_updateInfoRender);
						else
						{
							renderView->clear(
								render::CfColor | render::CfDepth | render::CfStencil,
								&m_backgroundColor,
								1.0f,
								0
							);
						}

						renderView->end();
					}
					if (renderView->begin(render::EtRight))
					{
						if (m_stateRender)
							m_stateRender->render(m_frameRender, render::EtRight, m_updateInfoRender);
						else
						{
							renderView->clear(
								render::CfColor | render::CfDepth | render::CfStencil,
								&m_backgroundColor,
								1.0f,
								0
							);
						}

						renderView->end();
					}
					renderView->present();
				}
			}
			else
			{
				// Yield render thread.
				m_threadRender->sleep(100);
			}

			double renderEnd = m_timer.getElapsedTime();
			m_renderDuration = float(renderEnd - renderBegin);

			m_renderServer->setFrameRate(int32_t(1.0f / m_renderDuration));
			m_stateRender = 0;
		}

		// Frame finished.
		m_signalRenderFinish.set();
	}
}

	}
}

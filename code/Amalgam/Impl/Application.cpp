#include "Core/Library/Library.h"
#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Memory/Alloc.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringArray.h"
#include "Core/Settings/Settings.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/JobManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Database/Database.h"
#include "Database/Events/EvtInstanceCommitted.h"
#include "Online/ISessionManager.h"
#include "Physics/PhysicsManager.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Resource/IResourceManager.h"
#include "Amalgam/Actions/ActivationAction.h"
#include "Amalgam/Actions/FullScreenAction.h"
#include "Amalgam/Actions/ReconfiguredAction.h"
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
#include "Amalgam/Impl/WorldServer.h"
#include "Amalgam/IState.h"
#include "Amalgam/IStateFactory.h"

// Static symbols.
#if defined(T_STATIC)
#	include <Animation/Animation/Animation.h>
#	include <Animation/Animation/StateNodeAnimation.h>
#	include <Animation/Animation/StatePoseControllerData.h>
#	include <Animation/IK/IKPoseControllerData.h>
#	include <Animation/PathEntity/PathEntityData.h>
#	include <Animation/RagDoll/RagDollPoseControllerData.h>
#	include <Animation/AnimatedMeshEntityData.h>
#	include <Core/Settings/PropertyBoolean.h>
#	include <Core/Settings/PropertyColor.h>
#	include <Core/Settings/PropertyFloat.h>
#	include <Core/Settings/PropertyGroup.h>
#	include <Core/Settings/PropertyInteger.h>
#	include <Core/Settings/PropertyString.h>
#	include <Core/Settings/PropertyStringArray.h>
#	include <Database/Compact/CompactDatabase.h>
#	include <Database/Local/LocalDatabase.h>
#	include <Database/Remote/Client/RemoteDatabase.h>
#	include <Input/Binding/CombinedInputSourceData.h>
#	include <Input/Binding/ConstantInputSourceData.h>
#	include <Input/Binding/GenericInputSourceData.h>
#	include <Input/Binding/KeyboardInputSourceData.h>
#	include <Input/Binding/IfInclusiveExclusive.h>
#	include <Input/Binding/InBoolean.h>
#	include <Input/Binding/InCombine.h>
#	include <Input/Binding/InConst.h>
#	include <Input/Binding/InHysteresis.h>
#	include <Input/Binding/InPulse.h>
#	include <Input/Binding/InReadValue.h>
#	include <Input/Binding/InRemapAxis.h>
#	include <Input/Binding/InThreshold.h>
#	include <Input/Binding/InTrigger.h>
#	include <Mesh/MeshEntityData.h>
#	include <Mesh/Composite/CompositeMeshEntityData.h>
#	include <Mesh/Blend/BlendMeshResource.h>
#	include <Mesh/Indoor/IndoorMeshResource.h>
#	include <Mesh/Instance/InstanceMeshResource.h>
#	include <Mesh/Partition/PartitionMeshResource.h>
#	include <Mesh/Partition/OctreePartitionData.h>
#	include <Mesh/Skinned/SkinnedMeshResource.h>
#	include <Mesh/Static/StaticMeshResource.h>
#	include <Mesh/Stream/StreamMeshResource.h>
#	include <Script/Lua/ScriptResourceLua.h>
#	include <Sound/StaticSoundResource.h>
#	include <Sound/StreamSoundResource.h>
#	include <Sound/Decoders/FlacStreamDecoder.h>
#	include <Sound/Decoders/Mp3StreamDecoder.h>
#	include <Sound/Decoders/OggStreamDecoder.h>
#	include <Sound/Decoders/WavStreamDecoder.h>
#	include <Sound/Resound/BankResource.h>
#	include <Sound/Resound/MuteGrain.h>
#	include <Sound/Resound/PlayGrain.h>
#	include <Sound/Resound/RandomGrain.h>
#	include <Sound/Resound/RepeatGrain.h>
#	include <Sound/Resound/SequenceGrain.h>
#	include <Spray/Modifiers/DragModifier.h>
#	include <Spray/Modifiers/GravityModifier.h>
#	include <Spray/Modifiers/IntegrateModifier.h>
#	include <Spray/Modifiers/PlaneCollisionModifier.h>
#	include <Spray/Modifiers/SizeModifier.h>
#	include <Spray/Modifiers/VortexModifier.h>
#	include <Spray/Sources/BoxSource.h>
#	include <Spray/Sources/ConeSource.h>
#	include <Spray/Sources/DiscSource.h>
#	include <Spray/Sources/PointSetSource.h>
#	include <Spray/Sources/PointSource.h>
#	include <Spray/Sources/QuadSource.h>
#	include <Spray/Sources/SphereSource.h>
#	include <Spray/Sources/SplineSource.h>
#	include <Theater/TheaterControllerData.h>
#	include <World/Entity/NullEntityData.h>
#	include <World/Forward/WorldRendererForward.h>
#	include <World/PostProcess/PostProcessDefineTarget.h>
#	include <World/PostProcess/PostProcessStepBlur.h>
#	include <World/PostProcess/PostProcessStepBokeh.h>
#	include <World/PostProcess/PostProcessStepChain.h>
#	include <World/PostProcess/PostProcessStepLuminance.h>
#	include <World/PostProcess/PostProcessStepRepeat.h>
#	include <World/PostProcess/PostProcessStepSetTarget.h>
#	include <World/PostProcess/PostProcessStepSimple.h>
#	include <World/PostProcess/PostProcessStepSmProj.h>
#	include <World/PostProcess/PostProcessStepSsao.h>
#	include <World/PostProcess/PostProcessStepSwapTargets.h>
#	include <World/PreLit/WorldRendererPreLit.h>
#endif

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

const int32_t c_databasePollInterval = 5;
const uint32_t c_simulationFrequency = 60;
const float c_simulationDeltaTime = 1.0f / c_simulationFrequency;
#if defined(_PS3)
const int32_t c_maxSimulationUpdates = 2;
#else
const int32_t c_maxSimulationUpdates = 4;
#endif
const float c_maxDeltaTime = 1.0f / 10.0f;
const float c_minDeltaTime = 0.0f;
#if defined(__APPLE__)
const float c_deltaTimeFilterCoeff = 0.1f;
#else
const float c_deltaTimeFilterCoeff = 0.9f;
#endif

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.Application", Application, IApplication)

Application::Application()
:	m_threadDatabase(0)
,	m_threadRender(0)
,	m_renderViewActive(true)
,	m_renderViewFullScreen(false)
,	m_waitRenderFinishInterval(0)
,	m_frameBuild(0)
,	m_frameRender(0)
,	m_stateRender(0)
#if T_MEASURE_PERFORMANCE
,	m_fps(0.0f)
#endif
{
	m_backgroundColor[0] = 0.0f;
	m_backgroundColor[1] = 0.0f;
	m_backgroundColor[2] = 0.0f;
	m_backgroundColor[3] = 0.0f;
}

bool Application::create(
	const Settings* defaultSettings,
	Settings* settings,
	online::ISessionManagerProvider* sessionManagerProvider,
	IStateFactory* stateFactory,
	void* nativeWindowHandle
)
{
	std::wstring targetManagerHost = settings->getProperty< PropertyString >(L"Amalgam.TargetManager/Host");
	int32_t targetManagerPort = settings->getProperty< PropertyInteger >(L"Amalgam.TargetManager/Port");
	Guid targetManagerId = Guid(settings->getProperty< PropertyString >(L"Amalgam.TargetManager/Id"));
	if (!targetManagerHost.empty() && targetManagerPort && targetManagerId.isValid())
	{
		m_targetManagerConnection = new TargetManagerConnection();
		if (!m_targetManagerConnection->connect(targetManagerHost, targetManagerPort, targetManagerId))
		{
			log::warning << L"Unable to connect to Target Manager; unable to debug" << Endl;
			m_targetManagerConnection = 0;
		}
	}

#if !defined(T_STATIC)
	std::vector< std::wstring > modules = settings->getProperty< PropertyStringArray >(L"Amalgam.Modules");
	m_libraries.resize(modules.size());
	for (uint32_t i = 0; i < modules.size(); ++i)
	{
		if (!m_libraries[i].open(modules[i]))
		{
			log::error << L"Application failed; unable to load module \"" << modules[i] << L"\"" << Endl;
			return false;
		}
	}
#else
	// Ensure symbols are linked.
	T_FORCE_LINK_REF(PropertyBoolean);
	T_FORCE_LINK_REF(PropertyColor);
	T_FORCE_LINK_REF(PropertyFloat);
	T_FORCE_LINK_REF(PropertyGroup);
	T_FORCE_LINK_REF(PropertyInteger);
	T_FORCE_LINK_REF(PropertyString);
	T_FORCE_LINK_REF(PropertyStringArray);
	T_FORCE_LINK_REF(db::CompactDatabase);
	T_FORCE_LINK_REF(db::LocalDatabase);
	T_FORCE_LINK_REF(db::RemoteDatabase);
	T_FORCE_LINK_REF(animation::Animation);
	T_FORCE_LINK_REF(animation::AnimatedMeshEntityData);
	T_FORCE_LINK_REF(animation::IKPoseControllerData);
	T_FORCE_LINK_REF(animation::PathEntityData);
	T_FORCE_LINK_REF(animation::RagDollPoseControllerData);
	T_FORCE_LINK_REF(animation::StateNodeAnimation);
	T_FORCE_LINK_REF(animation::StatePoseControllerData);
	T_FORCE_LINK_REF(input::CombinedInputSourceData);
	T_FORCE_LINK_REF(input::ConstantInputSourceData);
	T_FORCE_LINK_REF(input::GenericInputSourceData);
	T_FORCE_LINK_REF(input::KeyboardInputSourceData);
	T_FORCE_LINK_REF(input::IfInclusiveExclusive);
	T_FORCE_LINK_REF(input::InBoolean);
	T_FORCE_LINK_REF(input::InCombine);
	T_FORCE_LINK_REF(input::InConst);
	T_FORCE_LINK_REF(input::InHysteresis);
	T_FORCE_LINK_REF(input::InPulse);
	T_FORCE_LINK_REF(input::InReadValue);
	T_FORCE_LINK_REF(input::InRemapAxis);
	T_FORCE_LINK_REF(input::InThreshold);
	T_FORCE_LINK_REF(input::InTrigger);
	T_FORCE_LINK_REF(mesh::CompositeMeshEntityData);
	T_FORCE_LINK_REF(mesh::MeshEntityData);
	T_FORCE_LINK_REF(mesh::BlendMeshResource);
	T_FORCE_LINK_REF(mesh::IndoorMeshResource);
	T_FORCE_LINK_REF(mesh::InstanceMeshResource);
	T_FORCE_LINK_REF(mesh::PartitionMeshResource);
	T_FORCE_LINK_REF(mesh::OctreePartitionData);
	T_FORCE_LINK_REF(mesh::SkinnedMeshResource);
	T_FORCE_LINK_REF(mesh::StaticMeshResource);
	T_FORCE_LINK_REF(mesh::StreamMeshResource);
	T_FORCE_LINK_REF(script::ScriptResourceLua);
	T_FORCE_LINK_REF(sound::StaticSoundResource);
	T_FORCE_LINK_REF(sound::StreamSoundResource);
	T_FORCE_LINK_REF(sound::BankResource);
	T_FORCE_LINK_REF(sound::MuteGrain);
	T_FORCE_LINK_REF(sound::PlayGrain);
	T_FORCE_LINK_REF(sound::RandomGrain);
	T_FORCE_LINK_REF(sound::RepeatGrain);
	T_FORCE_LINK_REF(sound::SequenceGrain);
	T_FORCE_LINK_REF(sound::FlacStreamDecoder);
	T_FORCE_LINK_REF(sound::Mp3StreamDecoder);
	T_FORCE_LINK_REF(sound::OggStreamDecoder);
	T_FORCE_LINK_REF(sound::WavStreamDecoder);
	T_FORCE_LINK_REF(spray::DragModifier);
	T_FORCE_LINK_REF(spray::GravityModifier);
	T_FORCE_LINK_REF(spray::IntegrateModifier);
	T_FORCE_LINK_REF(spray::PlaneCollisionModifier);
	T_FORCE_LINK_REF(spray::SizeModifier);
	T_FORCE_LINK_REF(spray::VortexModifier);
	T_FORCE_LINK_REF(spray::BoxSource);
	T_FORCE_LINK_REF(spray::ConeSource);
	T_FORCE_LINK_REF(spray::DiscSource);
	T_FORCE_LINK_REF(spray::PointSetSource);
	T_FORCE_LINK_REF(spray::PointSource);
	T_FORCE_LINK_REF(spray::QuadSource);
	T_FORCE_LINK_REF(spray::SphereSource);
	T_FORCE_LINK_REF(spray::SplineSource);
	T_FORCE_LINK_REF(world::NullEntityData);
	T_FORCE_LINK_REF(theater::TheaterControllerData);
	T_FORCE_LINK_REF(world::WorldRendererForward);
	T_FORCE_LINK_REF(world::WorldRendererPreLit);
	T_FORCE_LINK_REF(world::PostProcessDefineTarget);
	T_FORCE_LINK_REF(world::PostProcessStepBlur);
	T_FORCE_LINK_REF(world::PostProcessStepBokeh);
	T_FORCE_LINK_REF(world::PostProcessStepChain);
	T_FORCE_LINK_REF(world::PostProcessStepLuminance);
	T_FORCE_LINK_REF(world::PostProcessStepRepeat);
	T_FORCE_LINK_REF(world::PostProcessStepSetTarget);
	T_FORCE_LINK_REF(world::PostProcessStepSimple);
	T_FORCE_LINK_REF(world::PostProcessStepSmProj);
	T_FORCE_LINK_REF(world::PostProcessStepSsao);
	T_FORCE_LINK_REF(world::PostProcessStepSwapTargets);
#endif

	m_stateManager = new StateManager();

	if (sessionManagerProvider)
	{
		log::debug << L"Creating online server..." << Endl;
		m_onlineServer = new OnlineServer();
		if (!m_onlineServer->create(sessionManagerProvider))
			return false;
	}

	log::debug << L"Creating resource server..." << Endl;
	m_resourceServer = new ResourceServer();
	if (!m_resourceServer->create())
		return false;

	log::debug << L"Creating render server..." << Endl;
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

	log::debug << L"Creating input server..." << Endl;
	m_inputServer = new InputServer();
	if (!m_inputServer->create(defaultSettings, settings, nativeWindowHandle))
		return false;

	log::debug << L"Creating physics server..." << Endl;
	m_physicsServer = new PhysicsServer();
	if (!m_physicsServer->create(settings, c_simulationDeltaTime))
		return false;

	log::debug << L"Creating script server..." << Endl;
	m_scriptServer = new ScriptServer();
	if (!m_scriptServer->create(settings, m_targetManagerConnection != 0))
		return false;

	log::debug << L"Creating world server..." << Endl;
	m_worldServer = new WorldServer();
	if (!m_worldServer->create(settings, m_renderServer, m_resourceServer))
		return false;

	log::debug << L"Creating audio server..." << Endl;
	m_audioServer = new AudioServer();
	if (!m_audioServer->create(settings))
		return false;

	log::debug << L"Creating database..." << Endl;
	m_database = new db::Database ();
	std::wstring connectionString = settings->getProperty< PropertyString >(L"Amalgam.Database");
	if (!m_database->open(connectionString))
	{
		log::error << L"Application failed; unable to open database \"" << connectionString << L"\"" << Endl;
		return false;
	}

	log::debug << L"Creating environment..." << Endl;
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

	m_audioServer->createResourceFactories(m_environment);
	m_inputServer->createResourceFactories(m_environment);
	m_physicsServer->createResourceFactories(m_environment);
	m_renderServer->createResourceFactories(m_environment);
	m_resourceServer->createResourceFactories(m_environment);
	m_scriptServer->createResourceFactories(m_environment);
	m_worldServer->createResourceFactories(m_environment);

	m_physicsServer->createEntityFactories(m_environment);
	m_worldServer->createEntityFactories(m_environment);

	if (settings->getProperty< PropertyBoolean >(L"Amalgam.DatabaseThread", false))
	{
		m_threadDatabase = ThreadManager::getInstance().create(makeFunctor(this, &Application::threadDatabase), L"Database events");
		if (m_threadDatabase)
			m_threadDatabase->start(Thread::Highest);
	}

	log::debug << L"Creating initial state..." << Endl;
	Ref< IState > state = stateFactory->create(m_environment);
	if (!state)
	{
		log::error << L"Application failed; unable to create initial state" << Endl;
		return false;
	}

	log::info << L"Application started successfully; enter initial state..." << Endl;

	m_timer.start();
	m_stateManager->enter(state);

	m_renderViewFullScreen = m_renderServer->getRenderView()->isFullScreen();

	log::info << L"Initial state ready; enter main loop..." << Endl;

	if (settings->getProperty< PropertyBoolean >(L"Amalgam.RenderThread", true))
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

	m_settings = settings;
	return true;
}

void Application::destroy()
{
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
	
	for (std::vector< Library >::iterator i = m_libraries.begin(); i != m_libraries.end(); ++i)
		i->close();
}

bool Application::update()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lockUpdate);
	Ref< IState > currentState;

	if (!m_renderServer->getRenderSystem()->handleMessages())
		return false;

	// Perform reconfiguration if required.
	if (m_environment->shouldReconfigure())
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
			ReconfiguredAction configuredAction(result);
			currentState->take(&configuredAction);
		}
	}

	// Update render server.
	m_renderServer->update(m_settings);

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

		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lockRender);
			T_FATAL_ASSERT (m_stateRender == 0);
			m_stateManager->performTransition();
			m_updateInfo.m_frameDeltaTime = c_simulationDeltaTime;
			m_updateInfo.m_simulationTime = 0.0f;
			m_updateInfo.m_stateTime = 0.0f;

			// Assume state's active from start.
			m_renderViewActive = true;
		};
		
		// Dump resource statistics.
#if defined(_DEBUG)
		m_resourceServer->dumpStatistics();
#endif
	}

	// Update scripting language runtime.
	m_scriptServer->update();

	if ((currentState = m_stateManager->getCurrent()) != 0)
	{
		// Check render active state; notify application when changes.
		bool renderViewActive = m_renderServer->getRenderView()->isActive();
		if (renderViewActive != m_renderViewActive)
		{
			ActivationAction activationAction(renderViewActive);
			currentState->take(&activationAction);
			m_renderViewActive = renderViewActive;
		}

		bool renderViewFullScreen = m_renderServer->getRenderView()->isFullScreen();
		if (renderViewFullScreen != m_renderViewFullScreen)
		{
			FullScreenAction fullScreenAction(renderViewFullScreen);
			currentState->take(&fullScreenAction);
			m_renderViewFullScreen = renderViewFullScreen;
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
		m_audioServer->update(m_updateInfo.m_frameDeltaTime, m_renderViewActive);

		// Update rumble.
		m_inputServer->updateRumble(m_updateInfo.m_frameDeltaTime, m_updateControl.m_pause);

		// Update active state; fixed time step if physics manager is available.
		double updateDuration = 0.0;
		double physicsDuration = 0.0;
		double inputDuration = 0.0;
		uint32_t updateCount = 0;

		physics::PhysicsManager* physicsManager = m_physicsServer->getPhysicsManager();
		if (physicsManager && !m_updateControl.m_pause)
		{
			m_updateInfo.m_simulationDeltaTime = c_simulationDeltaTime;
			m_updateInfo.m_simulationFrequency = c_simulationFrequency;

			UpdateInfo updateInfo = m_updateInfo;
			float simulationEndTime = m_updateInfo.m_stateTime;

			int32_t stepCount = int32_t((simulationEndTime - m_updateInfo.m_simulationTime) / c_simulationDeltaTime);
			if (stepCount > c_maxSimulationUpdates)
				stepCount = c_maxSimulationUpdates;

			// Execute fixed update(s).
			for (int32_t i = 0; i < stepCount; ++i, m_updateInfo.m_simulationTime += c_simulationDeltaTime)
			{
				// If we're doing multiple updates per frame then we're rendering bound; so in order
				// to keep input updating periodically we need to make sure we wait a bit, as long
				// as we don't collide with end-of-rendering.
				if (
					m_threadRender &&
					m_waitRenderFinishInterval > 0 &&
					i > 0
				)
				{
					if (m_signalRenderFinish.wait(m_waitRenderFinishInterval))
					{
						// Rendering of last frame has already finished; we need to decrease interval time
						// as we don't want rendering to be finished before the updates has.
						--m_waitRenderFinishInterval;
						break;
					}
				}

				double physicsTimeStart = m_timer.getElapsedTime();
				m_physicsServer->update();
				double physicsTimeEnd = m_timer.getElapsedTime();
				physicsDuration += physicsTimeEnd - physicsTimeStart;

				// Update input.
				double inputTimeStart = m_timer.getElapsedTime();
				m_inputServer->update(m_updateInfo.m_simulationDeltaTime, inputEnabled);
				double inputTimeEnd = m_timer.getElapsedTime();
				inputDuration += inputTimeEnd - inputTimeStart;

				// Update current state for each simulation tick.
				double updateTimeStart = m_timer.getElapsedTime();
				bool substep = ((m_updateInfo.m_simulationTime + c_simulationDeltaTime) < simulationEndTime);
				IState::UpdateResult result = currentState->update(m_stateManager, m_updateControl, m_updateInfo, substep);
				double updateTimeEnd = m_timer.getElapsedTime();
				updateDuration += updateTimeEnd - updateTimeStart;
				updateCount++;

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
		}
		else
		{
			// Update input.
			m_inputServer->update(m_updateInfo.m_frameDeltaTime, inputEnabled);

			// No physics; update in same rate as rendering.
			m_updateInfo.m_simulationTime = m_updateInfo.m_stateTime;
			m_updateInfo.m_simulationDeltaTime = m_updateInfo.m_frameDeltaTime;
			m_updateInfo.m_simulationFrequency = uint32_t(1.0f / m_updateInfo.m_frameDeltaTime);

			double updateTimeStart = m_timer.getElapsedTime();
			IState::UpdateResult updateResult = currentState->update(m_stateManager, m_updateControl, m_updateInfo, false);
			double updateTimeEnd = m_timer.getElapsedTime();
			updateDuration += updateTimeEnd - updateTimeStart;
			updateCount++;

			if (updateResult == IState::UrExit || updateResult == IState::UrFailed)
			{
				// Ensure render thread is finished before we leave.
				if (m_threadRender)
					m_signalRenderFinish.wait(1000);

				return false;
			}
		}

		// Leave if state has been changed; no need to render current state.
		if (m_stateManager->getNext())
			return true;

		// Build frame.
		double buildTimeStart = m_timer.getElapsedTime();
		IState::BuildResult buildResult = currentState->build(m_frameBuild, m_updateInfo);
		double buildTimeEnd = m_timer.getElapsedTime();

		if (buildResult == IState::BrOk || buildResult == IState::BrNothing)
		{
			if (m_threadRender)
			{
				// Synchronize with render thread and issue another rendering.
				double waitBegin = m_timer.getElapsedTime();
				bool renderFinished = m_signalRenderFinish.wait(1000);
				double waitEnd = m_timer.getElapsedTime();

				if (renderFinished)
				{
					// If we waited for rendering longer than a update cycle then
					// we increase interval time.
					if (updateCount > 1)
					{
						if ((waitEnd - waitBegin) > (physicsDuration + inputDuration + updateDuration) / updateCount)
							++m_waitRenderFinishInterval;
					}

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
									m_backgroundColor,
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
									m_backgroundColor,
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
									m_backgroundColor,
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

		m_updateInfo.m_totalTime += m_updateInfo.m_frameDeltaTime;
		m_updateInfo.m_stateTime += m_updateInfo.m_frameDeltaTime;
		m_updateInfo.m_frame++;

#if T_MEASURE_PERFORMANCE
		// Calculate frame rate.
		m_fps = m_fps * 0.9f + (1.0f / m_updateInfo.m_frameDeltaTime) * 0.1f;

		// Publish performance to target manager.
		if (m_targetManagerConnection)
		{
			render::RenderViewStatistics statistics;
			m_renderServer->getRenderView()->getStatistics(statistics);

			TargetPerformance performance;
			performance.time = m_updateInfo.m_totalTime;
			performance.fps = m_fps;
			if (updateCount > 0)
			{
				performance.update = float(updateDuration / updateCount);
				performance.physics = float(physicsDuration / updateCount);
				performance.input = float(inputDuration / updateCount);
			}
			performance.build = float(buildTimeEnd - buildTimeStart);
			performance.render = float(statistics.duration);
			performance.memInUse = Alloc::allocated();
			performance.heapObjects = Object::getHeapObjectCount();
			m_targetManagerConnection->setPerformance(performance);
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
		{
			// Update target manager connection.
			if (m_targetManagerConnection)
				m_targetManagerConnection->update();

			m_threadDatabase->sleep(100);
		}

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
					resourceManager->update(*i, true);
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
								m_backgroundColor,
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
								m_backgroundColor,
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
								m_backgroundColor,
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

			m_stateRender = 0;
		}

		// Frame finished.
		m_signalRenderFinish.set();
	}
}

	}
}

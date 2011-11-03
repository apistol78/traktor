#ifndef traktor_amalgam_Application_H
#define traktor_amalgam_Application_H

#include "Core/Library/Library.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/Signal.h"
#include "Core/Timer/Timer.h"
#include "Amalgam/IApplication.h"
#include "Amalgam/Impl/UpdateControl.h"
#include "Amalgam/Impl/UpdateInfo.h"

namespace traktor
{

class Settings;
class Thread;

	namespace db
	{

class Database;

	}

	namespace online
	{

class ISessionManagerProvider;

	}

	namespace amalgam
	{

// \fixme Move to configuration file.
#define T_MEASURE_PERFORMANCE 1

class AudioServer;
class InputServer;
class OnlineServer;
class PhysicsServer;
class RenderServer;
class ResourceServer;
class ScriptServer;
class WorldServer;

class Environment;
class IState;
class IStateFactory;
class StateManager;
class TargetManagerConnection;

/*! \brief Amalgam application implementation. */
class Application : public IApplication
{
	T_RTTI_CLASS;

public:
	Application();

	bool create(
		const Settings* defaultSettings,
		Settings* settings,
		online::ISessionManagerProvider* sessionManagerProvider,
		IStateFactory* stateFactory,
		void* nativeWindowHandle
	);

	void destroy();

	bool update();

	virtual Ref< IEnvironment > getEnvironment();

	virtual Ref< IStateManager > getStateManager();

private:
	Ref< Settings > m_settings;
	std::vector< Library > m_libraries;
	Ref< TargetManagerConnection > m_targetManagerConnection;
	Ref< db::Database > m_database;
	Ref< AudioServer > m_audioServer;
	Ref< InputServer > m_inputServer;
	Ref< OnlineServer > m_onlineServer;
	Ref< PhysicsServer > m_physicsServer;
	Ref< RenderServer > m_renderServer;
	Ref< ResourceServer > m_resourceServer;
	Ref< ScriptServer > m_scriptServer;
	Ref< WorldServer > m_worldServer;
	Ref< Environment > m_environment;
	Ref< StateManager > m_stateManager;
	Semaphore m_lockUpdate;
	Thread* m_threadDatabase;
	Thread* m_threadRender;
	Timer m_timer;
	UpdateControl m_updateControl;
	UpdateInfo m_updateInfo;
	bool m_renderViewActive;
	float m_backgroundColor[4];
	float m_updateDuration;
	float m_buildDuration;
	float m_renderDuration;
	uint32_t m_renderCollisions;
	Semaphore m_lockRender;
	Signal m_signalRenderBegin;
	Signal m_signalRenderFinish;
	uint32_t m_frameBuild;
	uint32_t m_frameRender;
	Ref< IState > m_stateRender;
	UpdateInfo m_updateInfoRender;

#if T_MEASURE_PERFORMANCE
	float m_fps;
#endif

	bool updateInputDevices();

	void threadDatabase();

	void threadRender();
};

	}
}

#endif	// traktor_amalgam_Application_H

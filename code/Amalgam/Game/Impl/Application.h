#ifndef traktor_amalgam_Application_H
#define traktor_amalgam_Application_H

#include "Amalgam/Game/FrameProfiler.h"
#include "Amalgam/Game/IApplication.h"
#include "Amalgam/Game/UpdateControl.h"
#include "Amalgam/Game/UpdateInfo.h"
#include "Core/Platform.h"
#include "Core/RefArray.h"
#include "Core/Library/Library.h"
#include "Core/Math/Color4f.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/Signal.h"
#include "Core/Timer/Timer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_GAME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class PropertyGroup;
class Thread;

	namespace db
	{

class Database;

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
class IRuntimePlugin;
class IState;
class StateManager;
class TargetManagerConnection;

/*! \brief Amalgam application implementation.
 * \ingroup Amalgam
 */
class T_DLLCLASS Application : public IApplication
{
	T_RTTI_CLASS;

public:
	Application();

	bool create(
		const PropertyGroup* defaultSettings,
		PropertyGroup* settings,
		const SystemApplication& sysapp,
		const SystemWindow* syswin
	);

	void destroy();

	bool update();

	void suspend();

	void resume();

	virtual Ref< IEnvironment > getEnvironment() T_OVERRIDE T_FINAL;

	virtual Ref< IStateManager > getStateManager() T_OVERRIDE T_FINAL;

private:
	Ref< PropertyGroup > m_settings;
	RefArray< Library > m_libraries;
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
	RefArray< IRuntimePlugin > m_plugins;
	Semaphore m_lockUpdate;
#if !defined(__EMSCRIPTEN__)
	Thread* m_threadDatabase;
	Thread* m_threadRender;
#endif
	Timer m_timer;
	FrameProfiler m_frameProfiler;
	int32_t m_maxSimulationUpdates;
	int32_t m_deltaTimeError;
	UpdateControl m_updateControl;
	UpdateInfo m_updateInfo;
	bool m_renderViewActive;
	Color4f m_backgroundColor;
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

	void pollDatabase();

#if !defined(__EMSCRIPTEN__)

	void threadDatabase();

	void threadRender();

#endif
};

	}
}

#endif	// traktor_amalgam_Application_H

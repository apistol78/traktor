/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Runtime/IApplication.h"
#include "Runtime/UpdateControl.h"
#include "Runtime/UpdateInfo.h"
#include "Runtime/Target/TargetPerformance.h"
#include "Core/Platform.h"
#include "Core/RefArray.h"
#include "Core/Library/Library.h"
#include "Core/Math/Color4f.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/Signal.h"
#include "Core/Timer/Timer.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
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

	namespace runtime
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

/*! Runtime application implementation.
 * \ingroup Runtime
 */
class T_DLLCLASS Application : public IApplication
{
	T_RTTI_CLASS;

public:
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

	virtual IEnvironment* getEnvironment() override final;

	virtual IStateManager* getStateManager() override final;

private:
	Ref< PropertyGroup > m_settings;
	RefArray< Library > m_libraries;
	RefArray< IRuntimePlugin > m_plugins;
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
	Thread* m_threadDatabase = nullptr;
	Thread* m_threadRender = nullptr;
	Timer m_timer;
	int32_t m_maxSimulationUpdates = 1;
	UpdateControl m_updateControl;
	UpdateInfo m_updateInfo;
	bool m_renderViewActive = true;
	Color4f m_backgroundColor = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	float m_updateDuration = 0.0f;
	float m_buildDuration = 0.0f;
	float m_renderCpuDurations[2] = { 0.0f, 0.0f };
	float m_renderGpuDuration = 0.0f;
	int32_t m_renderGpuDurationQuery = -1;
	uint32_t m_renderCollisions = 0;
	Semaphore m_lockRender;
	Signal m_signalRenderBegin;
	Signal m_signalRenderFinish;
	uint32_t m_frameBuild = 0;
	uint32_t m_frameRender = 0;
	Ref< IState > m_stateRender;
	UpdateInfo m_updateInfoRender;
	render::RenderViewStatistics m_renderViewStats;
	TargetPerformance m_targetPerformance;

	bool updateInputDevices();

	void pollDatabase();

	void threadDatabase();

	void threadRender();
};

	}
}


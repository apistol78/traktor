/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/Impl/AudioServer.h"
#include "Runtime/Impl/Environment.h"
#include "Runtime/Impl/InputServer.h"
#include "Runtime/Impl/OnlineServer.h"
#include "Runtime/Impl/PhysicsServer.h"
#include "Runtime/Impl/RenderServer.h"
#include "Runtime/Impl/ResourceServer.h"
#include "Runtime/Impl/ScriptServer.h"
#include "Runtime/UpdateControl.h"
#include "Runtime/Impl/WorldServer.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.Environment", Environment, IEnvironment)

Environment::Environment(
	PropertyGroup* settings,
	db::Database* database,
	UpdateControl* control,
	AudioServer* audioServer,
	InputServer* inputServer,
	OnlineServer* onlineServer,
	PhysicsServer* physicsServer,
	RenderServer* renderServer,
	ResourceServer* resourceServer,
	ScriptServer* scriptServer,
	WorldServer* worldServer
)
:	m_settings(settings)
,	m_database(database)
,	m_control(control)
,	m_audioServer(audioServer)
,	m_inputServer(inputServer)
,	m_onlineServer(onlineServer)
,	m_physicsServer(physicsServer)
,	m_renderServer(renderServer)
,	m_resourceServer(resourceServer)
,	m_scriptServer(scriptServer)
,	m_worldServer(worldServer)
,	m_shouldReconfigure(false)
{
}

db::Database* Environment::getDatabase()
{
	return m_database;
}

UpdateControl* Environment::getControl()
{
	return m_control;
}

IAudioServer* Environment::getAudio()
{
	return m_audioServer;
}

IInputServer* Environment::getInput()
{
	return m_inputServer;
}

IOnlineServer* Environment::getOnline()
{
	return m_onlineServer;
}

IPhysicsServer* Environment::getPhysics()
{
	return m_physicsServer;
}

IRenderServer* Environment::getRender()
{
	return m_renderServer;
}

IResourceServer* Environment::getResource()
{
	return m_resourceServer;
}

IScriptServer* Environment::getScript()
{
	return m_scriptServer;
}

IWorldServer* Environment::getWorld()
{
	return m_worldServer;
}

PropertyGroup* Environment::getSettings()
{
	return m_settings;
}

bool Environment::reconfigure()
{
	m_shouldReconfigure = true;
	return true;
}

int32_t Environment::executeReconfigure()
{
	m_shouldReconfigure = false;

	int32_t result = CrUnaffected;

	if (m_audioServer && (result |= m_audioServer->reconfigure(m_settings)) == CrFailed)
		return CrFailed;
	if (m_inputServer && (result |= m_inputServer->reconfigure(m_settings)) == CrFailed)
		return CrFailed;
	if (m_onlineServer && (result |= m_onlineServer->reconfigure(m_settings)) == CrFailed)
		return CrFailed;
	if (m_physicsServer && (result |= m_physicsServer->reconfigure(m_settings)) == CrFailed)
		return CrFailed;
	if (m_renderServer && (result |= m_renderServer->reconfigure(this, m_settings)) == CrFailed)
		return CrFailed;
	if (m_resourceServer && (result |= m_resourceServer->reconfigure(m_settings)) == CrFailed)
		return CrFailed;
	if (m_scriptServer && (result |= m_scriptServer->reconfigure(m_settings)) == CrFailed)
		return CrFailed;
	if (m_worldServer && (result |= m_worldServer->reconfigure(m_settings)) == CrFailed)
		return CrFailed;

	return result;
}

	}
}

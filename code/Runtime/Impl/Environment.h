/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Runtime/IEnvironment.h"

namespace traktor
{

class PropertyGroup;

}

namespace traktor::runtime
{

class AudioServer;
class InputServer;
class OnlineServer;
class PhysicsServer;
class RenderServer;
class ResourceServer;
class ScriptServer;
class UpdateControl;
class WorldServer;

/*!
 * \ingroup Runtime
 */
class Environment : public IEnvironment
{
	T_RTTI_CLASS;

public:
	Environment(
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
	);

	virtual db::Database* getDatabase() override final;

	virtual UpdateControl* getControl() override final;

	virtual IAudioServer* getAudio() override final;

	virtual IInputServer* getInput() override final;

	virtual IOnlineServer* getOnline() override final;

	virtual IPhysicsServer* getPhysics() override final;

	virtual IRenderServer* getRender() override final;

	virtual IResourceServer* getResource() override final;

	virtual IScriptServer* getScript() override final;

	virtual IWorldServer* getWorld() override final;

	virtual PropertyGroup* getSettings() override final;

	virtual bool reconfigure() override final;

	int32_t executeReconfigure();

	bool shouldReconfigure() const { return m_shouldReconfigure; }

private:
	Ref< PropertyGroup > m_settings;
	Ref< db::Database > m_database;
	Ref< UpdateControl > m_control;
	Ref< AudioServer > m_audioServer;
	Ref< InputServer > m_inputServer;
	Ref< OnlineServer > m_onlineServer;
	Ref< PhysicsServer > m_physicsServer;
	Ref< RenderServer > m_renderServer;
	Ref< ResourceServer > m_resourceServer;
	Ref< ScriptServer > m_scriptServer;
	Ref< WorldServer > m_worldServer;
	bool m_shouldReconfigure;
};

}

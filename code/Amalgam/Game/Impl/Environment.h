#ifndef traktor_amalgam_Environment_H
#define traktor_amalgam_Environment_H

#include "Amalgam/Game/IEnvironment.h"

namespace traktor
{

class PropertyGroup;

	namespace amalgam
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

	virtual db::Database* getDatabase() T_OVERRIDE T_FINAL;

	virtual UpdateControl* getControl() T_OVERRIDE T_FINAL;

	virtual IAudioServer* getAudio() T_OVERRIDE T_FINAL;

	virtual IInputServer* getInput() T_OVERRIDE T_FINAL;

	virtual IOnlineServer* getOnline() T_OVERRIDE T_FINAL;

	virtual IPhysicsServer* getPhysics() T_OVERRIDE T_FINAL;

	virtual IRenderServer* getRender() T_OVERRIDE T_FINAL;

	virtual IResourceServer* getResource() T_OVERRIDE T_FINAL;

	virtual IScriptServer* getScript() T_OVERRIDE T_FINAL;

	virtual IWorldServer* getWorld() T_OVERRIDE T_FINAL;

	virtual PropertyGroup* getSettings() T_OVERRIDE T_FINAL;

	virtual bool reconfigure() T_OVERRIDE T_FINAL;

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
}

#endif	// traktor_amalgam_Environment_H

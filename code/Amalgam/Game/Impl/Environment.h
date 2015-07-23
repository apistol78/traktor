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

	virtual db::Database* getDatabase();

	virtual UpdateControl* getControl();

	virtual IAudioServer* getAudio();

	virtual IInputServer* getInput();

	virtual IOnlineServer* getOnline();

	virtual IPhysicsServer* getPhysics();

	virtual IRenderServer* getRender();

	virtual IResourceServer* getResource();

	virtual IScriptServer* getScript();

	virtual IWorldServer* getWorld();

	virtual PropertyGroup* getSettings();

	virtual bool reconfigure();

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

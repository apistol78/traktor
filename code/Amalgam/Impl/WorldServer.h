#ifndef traktor_amalgam_WorldServer_H
#define traktor_amalgam_WorldServer_H

#include "Amalgam/IWorldServer.h"

namespace traktor
{

class Settings;

	namespace scene
	{

class SceneFactory;

	}

	namespace amalgam
	{

class IEnvironment;
class IRenderServer;
class IResourceServer;

class WorldServer : public IWorldServer
{
	T_RTTI_CLASS;

public:
	bool create(const Settings* settings, IRenderServer* renderServer, IResourceServer* resourceServer);

	void destroy();

	void createResourceFactories(IEnvironment* environment);

	void createEntityFactories(IEnvironment* environment);

	int32_t reconfigure(const Settings* settings);

	virtual world::IEntityBuilder* getEntityBuilder();

	virtual Ref< world::WorldRenderer > createWorldRenderer(
		const world::WorldRenderSettings* worldRenderSettings,
		const world::WorldEntityRenderers* entityRenderers
	);

	virtual int32_t getFrameCount() const;

private:
	Ref< world::IEntityBuilder > m_entityBuilder;
	Ref< IRenderServer > m_renderServer;
	Ref< IResourceServer > m_resourceServer;
	Ref< scene::SceneFactory > m_sceneFactory;
};

	}
}

#endif	// traktor_amalgam_WorldServer_H

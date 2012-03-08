#ifndef traktor_amalgam_WorldServer_H
#define traktor_amalgam_WorldServer_H

#include "Amalgam/IWorldServer.h"

namespace traktor
{

class PropertyGroup;

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
	bool create(const PropertyGroup* settings, IRenderServer* renderServer, IResourceServer* resourceServer);

	void destroy();

	void createResourceFactories(IEnvironment* environment);

	void createEntityFactories(IEnvironment* environment);

	int32_t reconfigure(const PropertyGroup* settings);

	virtual void addEntityFactory(world::IEntityFactory* entityFactory);

	virtual void removeEntityFactory(world::IEntityFactory* entityFactory);

	virtual void addEntityRenderer(world::IEntityRenderer* entityRenderer);

	virtual void removeEntityRenderer(world::IEntityRenderer* entityRenderer);

	virtual world::IEntityBuilder* getEntityBuilder();

	virtual world::WorldEntityRenderers* getEntityRenderers();

	virtual Ref< world::IWorldRenderer > createWorldRenderer(const world::WorldRenderSettings* worldRenderSettings);

	virtual int32_t getFrameCount() const;

private:
	Ref< world::IEntityBuilder > m_entityBuilder;
	Ref< world::WorldEntityRenderers > m_entityRenderers;
	Ref< IRenderServer > m_renderServer;
	Ref< IResourceServer > m_resourceServer;
	Ref< scene::SceneFactory > m_sceneFactory;
};

	}
}

#endif	// traktor_amalgam_WorldServer_H

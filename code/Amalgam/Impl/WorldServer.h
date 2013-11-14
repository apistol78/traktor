#ifndef traktor_amalgam_WorldServer_H
#define traktor_amalgam_WorldServer_H

#include "Amalgam/IWorldServer.h"
#include "World/WorldRenderSettings.h"

namespace traktor
{

class PropertyGroup;

	namespace spray
	{

class EffectEntityRenderer;
class FeedbackManager;

	}

	namespace terrain
	{

class EntityRenderer;

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
	WorldServer();

	bool create(const PropertyGroup* settings, IRenderServer* renderServer, IResourceServer* resourceServer);

	void destroy();

	void createResourceFactories(IEnvironment* environment);

	void createEntityFactories(IEnvironment* environment);

	int32_t reconfigure(const PropertyGroup* settings);

	virtual void addEntityFactory(world::IEntityFactory* entityFactory);

	virtual void removeEntityFactory(world::IEntityFactory* entityFactory);

	virtual void addEntityRenderer(world::IEntityRenderer* entityRenderer);

	virtual void removeEntityRenderer(world::IEntityRenderer* entityRenderer);

	virtual const world::IEntityBuilder* getEntityBuilder();

	virtual world::WorldEntityRenderers* getEntityRenderers();

	virtual world::IEntityEventManager* getEntityEventManager();

	virtual spray::IFeedbackManager* getFeedbackManager();

	virtual Ref< world::IWorldRenderer > createWorldRenderer(
		const world::WorldRenderSettings* worldRenderSettings,
		const world::PostProcessSettings* postProcessSettings
	);

	virtual int32_t getFrameCount() const;

private:
	Ref< world::IEntityBuilder > m_entityBuilder;
	Ref< world::WorldEntityRenderers > m_entityRenderers;
	Ref< world::IEntityEventManager > m_eventManager;
	Ref< IRenderServer > m_renderServer;
	Ref< IResourceServer > m_resourceServer;
	Ref< spray::EffectEntityRenderer > m_effectEntityRenderer;
	Ref< spray::FeedbackManager > m_feedbackManager;
	Ref< terrain::EntityRenderer > m_terrainEntityRenderer;
	const TypeInfo* m_worldType;
	world::Quality m_shadowQuality;
	world::Quality m_ambientOcclusionQuality;
	world::Quality m_antiAliasQuality;
	world::Quality m_particleQuality;
	world::Quality m_oceanQuality;
};

	}
}

#endif	// traktor_amalgam_WorldServer_H

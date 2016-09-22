#ifndef traktor_amalgam_WorldServer_H
#define traktor_amalgam_WorldServer_H

#include "Amalgam/Game/IWorldServer.h"
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

/*! \brief
 * \ingroup Amalgam
 */
class WorldServer : public IWorldServer
{
	T_RTTI_CLASS;

public:
	WorldServer();

	bool create(const PropertyGroup* defaultSettings, const PropertyGroup* settings, IRenderServer* renderServer, IResourceServer* resourceServer);

	void destroy();

	void createResourceFactories(IEnvironment* environment);

	void createEntityFactories(IEnvironment* environment);

	int32_t reconfigure(const PropertyGroup* settings);

	virtual void addEntityFactory(world::IEntityFactory* entityFactory) T_OVERRIDE T_FINAL;

	virtual void removeEntityFactory(world::IEntityFactory* entityFactory) T_OVERRIDE T_FINAL;

	virtual void addEntityRenderer(world::IEntityRenderer* entityRenderer) T_OVERRIDE T_FINAL;

	virtual void removeEntityRenderer(world::IEntityRenderer* entityRenderer) T_OVERRIDE T_FINAL;

	virtual const world::IEntityBuilder* getEntityBuilder() T_OVERRIDE T_FINAL;

	virtual world::WorldEntityRenderers* getEntityRenderers() T_OVERRIDE T_FINAL;

	virtual world::IEntityEventManager* getEntityEventManager() T_OVERRIDE T_FINAL;

	virtual spray::IFeedbackManager* getFeedbackManager() T_OVERRIDE T_FINAL;

	virtual Ref< world::IWorldRenderer > createWorldRenderer(const world::WorldRenderSettings* worldRenderSettings) T_OVERRIDE T_FINAL;

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
	world::Quality m_imageProcessQuality;
	world::Quality m_particleQuality;
	world::Quality m_terrainQuality;
	world::Quality m_oceanQuality;
	float m_gamma;
	int32_t m_superSample;
};

	}
}

#endif	// traktor_amalgam_WorldServer_H

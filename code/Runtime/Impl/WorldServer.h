/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Runtime/IWorldServer.h"
#include "World/WorldRenderSettings.h"

namespace traktor
{

class PropertyGroup;

}

namespace traktor::spray
{

class EffectRenderer;
class FeedbackManager;

}

namespace traktor::terrain
{

class EntityRenderer;

}

namespace traktor::world
{

class EntityFactory;

}

namespace traktor::runtime
{

class IEnvironment;
class IRenderServer;
class IResourceServer;

/*! World server implementation.
 * \ingroup Runtime
 */
class WorldServer : public IWorldServer
{
	T_RTTI_CLASS;

public:
	bool create(const PropertyGroup* defaultSettings, const PropertyGroup* settings, IRenderServer* renderServer, IResourceServer* resourceServer);

	void destroy();

	void createResourceFactories(IEnvironment* environment);

	void createEntityFactories(IEnvironment* environment);

	void createEntityRenderers(IEnvironment* environment);

	int32_t reconfigure(const PropertyGroup* settings);

	virtual void addEntityFactory(world::IEntityFactory* entityFactory) override final;

	virtual void removeEntityFactory(world::IEntityFactory* entityFactory) override final;

	virtual void addEntityRenderer(world::IEntityRenderer* entityRenderer) override final;

	virtual void removeEntityRenderer(world::IEntityRenderer* entityRenderer) override final;

	virtual world::IEntityFactory* getEntityFactory() override final;

	virtual world::WorldEntityRenderers* getEntityRenderers() override final;

	virtual spray::IFeedbackManager* getFeedbackManager() override final;

	virtual Ref< world::IWorldRenderer > createWorldRenderer(const world::WorldRenderSettings* worldRenderSettings) override final;

private:
	Ref< world::EntityFactory > m_entityFactory;
	Ref< world::WorldEntityRenderers > m_entityRenderers;
	Ref< IRenderServer > m_renderServer;
	Ref< IResourceServer > m_resourceServer;
	Ref< spray::EffectRenderer > m_effectEntityRenderer;
	Ref< spray::FeedbackManager > m_feedbackManager;
	Ref< terrain::EntityRenderer > m_terrainEntityRenderer;
	const TypeInfo* m_worldType = nullptr;
	world::Quality m_motionBlurQuality = world::Quality::Medium;
	world::Quality m_shadowQuality = world::Quality::Medium;
	world::Quality m_reflectionsQuality = world::Quality::Medium;
	world::Quality m_ambientOcclusionQuality = world::Quality::Medium;
	world::Quality m_antiAliasQuality = world::Quality::Medium;
	world::Quality m_imageProcessQuality = world::Quality::Medium;
	world::Quality m_particleQuality = world::Quality::Medium;
	world::Quality m_terrainQuality = world::Quality::Medium;
	world::Quality m_oceanQuality = world::Quality::Medium;
	float m_gamma = 2.2f;
};

}

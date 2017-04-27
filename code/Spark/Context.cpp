/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Spark/Context.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Context", Context, Object)

Context::Context(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	sound::ISoundPlayer* soundPlayer
)
:	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
,	m_soundPlayer(soundPlayer)
{
}

resource::IResourceManager* Context::getResourceManager() const
{
	return m_resourceManager;
}

render::IRenderSystem* Context::getRenderSystem() const
{
	return m_renderSystem;
}

sound::ISoundPlayer* Context::getSoundPlayer() const
{
	return m_soundPlayer;
}

	}
}

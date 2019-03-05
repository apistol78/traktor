#include "Amalgam/IEnvironment.h"
#include "Amalgam/Engine/WorldLayer.h"
#include "Amalgam/Engine/WorldLayerData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Scene/Scene.h"
#include "Scene/SceneResource.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.WorldLayerData", 0, WorldLayerData, LayerData)

Ref< Layer > WorldLayerData::createInstance(Stage* stage, IEnvironment* environment) const
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	resource::Proxy< scene::Scene > scene;

	// Bind proxies to resource manager.
	if (!resourceManager->bind(m_scene, scene))
		return 0;

	// Create layer instance.
	return new WorldLayer(
		stage,
		m_name,
		m_permitTransition,
		environment,
		scene
	);
}

void WorldLayerData::serialize(ISerializer& s)
{
	LayerData::serialize(s);
	s >> resource::Member< scene::Scene >(L"scene", m_scene);
}

	}
}

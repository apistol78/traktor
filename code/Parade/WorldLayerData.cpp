#include "Amalgam/IEnvironment.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Parade/WorldLayer.h"
#include "Parade/WorldLayerData.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Scene/Scene.h"
#include "Scene/SceneResource.h"
#include "Script/IScriptContext.h"
#include "World/Entity/EntityData.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.parade.WorldLayerData", 0, WorldLayerData, LayerData)

Ref< Layer > WorldLayerData::createInstance(amalgam::IEnvironment* environment) const
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();

	// Create a copy of all immutable proxies.
	resource::Proxy< script::IScriptContext > script = m_script;
	resource::Proxy< scene::Scene > scene = m_scene;
	std::map< std::wstring, resource::Proxy< world::EntityData > > entities = m_entities;

	// Bind proxies to resource manager.
	if (
		!resourceManager->bind(script) ||
		!resourceManager->bind(scene)
	)
		return 0;

	for (std::map< std::wstring, resource::Proxy< world::EntityData > >::iterator i = entities.begin(); i != entities.end(); ++i)
	{
		if (!resourceManager->bind(i->second))
			return 0;
	}

	// Create layer instance.
	return new WorldLayer(
		m_name,
		environment,
		script,
		scene,
		entities
	);
}

bool WorldLayerData::serialize(ISerializer& s)
{
	if (!LayerData::serialize(s))
		return false;

	s >> resource::Member< scene::Scene, scene::SceneResource >(L"scene", m_scene);
	s >> MemberStlMap<
		std::wstring,
		resource::Proxy< world::EntityData >,
		MemberStlPair<
			std::wstring,
			resource::Proxy< world::EntityData >,
			Member< std::wstring >,
			resource::Member< world::EntityData >
		>
	>(L"entities", m_entities);

	return true;
}

	}
}

#include "Scene/SceneAsset.h"
#include "World/WorldRenderSettings.h"
#include "World/Entity/EntityInstance.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_EDITABLE_CLASS(L"traktor.scene.SceneAsset", SceneAsset, Serializable)

SceneAsset::SceneAsset()
:	m_worldRenderSettings(gc_new< world::WorldRenderSettings >())
{
}

void SceneAsset::setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings)
{
	m_worldRenderSettings = worldRenderSettings;
}

world::WorldRenderSettings* SceneAsset::getWorldRenderSettings() const
{
	return m_worldRenderSettings;
}

void SceneAsset::setInstance(world::EntityInstance* instance)
{
	m_instance = instance;
}

world::EntityInstance* SceneAsset::getInstance() const
{
	return m_instance;
}

bool SceneAsset::serialize(Serializer& s)
{
	s >> MemberRef< world::WorldRenderSettings >(L"worldRenderSettings", m_worldRenderSettings);
	s >> MemberRef< world::EntityInstance >(L"instance", m_instance);
	return true;
}

	}
}

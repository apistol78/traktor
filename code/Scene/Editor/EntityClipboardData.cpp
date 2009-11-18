#include "Scene/Editor/EntityClipboardData.h"
#include "World/Entity/EntityInstance.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.EntityClipboardData", EntityClipboardData, ISerializable)

void EntityClipboardData::addInstance(world::EntityInstance* instance)
{
	m_instances.push_back(instance);
}

const RefArray< world::EntityInstance >& EntityClipboardData::getInstances() const
{
	return m_instances;
}

bool EntityClipboardData::serialize(ISerializer& s)
{
	return s >> MemberRefArray< world::EntityInstance >(L"instances", m_instances);
}

	}
}

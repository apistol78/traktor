#include "Scene/Editor/EntityClipboardData.h"
#include "World/Entity/EntityInstance.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRefArray.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.scene.EntityClipboardData", EntityClipboardData, Serializable)

void EntityClipboardData::addInstance(world::EntityInstance* instance)
{
	m_instances.push_back(instance);
}

const RefArray< world::EntityInstance >& EntityClipboardData::getInstances() const
{
	return m_instances;
}

bool EntityClipboardData::serialize(Serializer& s)
{
	return s >> MemberRefArray< world::EntityInstance >(L"instances", m_instances);
}

	}
}

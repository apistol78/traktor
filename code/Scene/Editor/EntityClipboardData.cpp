#include "Scene/Editor/EntityClipboardData.h"
#include "World/Entity/EntityData.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.scene.EntityClipboardData", EntityClipboardData, Serializable)

void EntityClipboardData::addEntityData(world::EntityData* entityData)
{
	m_entityData.push_back(entityData);
}

const RefArray< world::EntityData >& EntityClipboardData::getEntityData() const
{
	return m_entityData;
}

bool EntityClipboardData::serialize(Serializer& s)
{
	return s >> MemberRefArray< world::EntityData >(L"entityData", m_entityData);
}

	}
}

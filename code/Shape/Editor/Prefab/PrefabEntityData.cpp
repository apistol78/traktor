#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Shape/Editor/Prefab/PrefabEntityData.h"
#include "World/Editor/EditorAttributesComponentData.h"
#include "World/Entity/GroupComponentData.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.shape.PrefabEntityData", 1, PrefabEntityData, world::EntityData)

PrefabEntityData::PrefabEntityData()
:	m_partitionMesh(false)
{
	setComponent(new world::EditorAttributesComponentData());
	setComponent(new world::GroupComponentData());
}

void PrefabEntityData::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion() >= 1);

	world::EntityData::serialize(s);

	s >> Member< bool >(L"partitionMesh", m_partitionMesh);
}

	}
}

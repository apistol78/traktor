#include "Ai/NavMesh.h"
#include "Ai/NavMeshEntityData.h"
#include "Core/Serialization/ISerializer.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace ai
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ai.NavMeshEntityData", 0, NavMeshEntityData, world::EntityData)

bool NavMeshEntityData::serialize(ISerializer& s)
{
	if (!world::EntityData::serialize(s))
		return false;

	s >> resource::Member< NavMesh >(L"navMesh", m_navMesh);
	return true;
}

	}
}

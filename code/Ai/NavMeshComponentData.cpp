#include "Ai/NavMesh.h"
#include "Ai/NavMeshComponentData.h"
#include "Core/Serialization/ISerializer.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace ai
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ai.NavMeshComponentData", 0, NavMeshComponentData, world::IEntityComponentData)

void NavMeshComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void NavMeshComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< NavMesh >(L"navMesh", m_navMesh);
}

	}
}

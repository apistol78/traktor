#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Physics/Editor/MeshAsset.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.physics.MeshAsset", 1, MeshAsset, editor::Asset)

MeshAsset::MeshAsset()
:	m_calculateConvexHull(true)
{
}

void MeshAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);

	if (s.getVersion() >= 1)
		s >> Member< bool >(L"calculateConvexHull", m_calculateConvexHull);
}

	}
}

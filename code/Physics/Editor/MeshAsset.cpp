#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStl.h"
#include "Physics/Editor/Material.h"
#include "Physics/Editor/MeshAsset.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.physics.MeshAsset", 4, MeshAsset, editor::Asset)

void MeshAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);

	if (s.getVersion() >= 4)
		s >> Member< std::wstring >(L"importFilter", m_importFilter);

	if (s.getVersion() >= 1)
		s >> Member< bool >(L"calculateConvexHull", m_calculateConvexHull);

	if (s.getVersion() >= 2)
		s >> Member< float >(L"margin", m_margin);

	if (s.getVersion< MeshAsset >() >= 3)
		s >> MemberStlMap< std::wstring, Guid >(L"materials", m_materials, AttributeType(type_of< Material >()));
}

	}
}

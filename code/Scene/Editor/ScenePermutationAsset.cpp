#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Scene/Editor/SceneAsset.h"
#include "Scene/Editor/ScenePermutationAsset.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.scene.ScenePermutationAsset", 0, ScenePermutationAsset, ISerializable)

void ScenePermutationAsset::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"scene", m_scene, AttributeType(type_of< SceneAsset >()));
	s >> MemberStlList< std::wstring >(L"includeLayers", m_includeLayers);
}

	}
}

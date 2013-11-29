#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Core/Serialization/MemberStl.h"
#include "Render/ITexture.h"
#include "Resource/Member.h"
#include "Scene/Editor/SceneAsset.h"
#include "Scene/Editor/ScenePermutationAsset.h"
#include "World/WorldRenderSettings.h"
#include "World/PostProcess/PostProcessSettings.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.scene.ScenePermutationAsset", 2, ScenePermutationAsset, ISerializable)

void ScenePermutationAsset::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"scene", m_scene, AttributeType(type_of< SceneAsset >()));
	s >> MemberStlList< std::wstring >(L"includeLayers", m_includeLayers);

	if (s.getVersion() >= 1)
	{
		s >> MemberRef< world::WorldRenderSettings >(L"overrideWorldRenderSettings", m_overrideWorldRenderSettings);
		s >> resource::Member< world::PostProcessSettings >(L"overridePostProcessSettings", m_overridePostProcessSettings);
	}

	if (s.getVersion() >= 2)
	{
		s >> MemberSmallMap< std::wstring, resource::Id< render::ITexture >, Member< std::wstring >, resource::Member< render::ITexture > >(L"overridePostProcessParams", m_overridePostProcessParams);
	}
}

	}
}

#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Core/Serialization/MemberStaticArray.h"
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
		namespace
		{

const wchar_t* c_PostProcessSettings_elementNames[] =
{
	L"disabled",
	L"low",
	L"medium",
	L"high",
	L"ultra"
};

		}

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.scene.ScenePermutationAsset", 3, ScenePermutationAsset, ISerializable)

void ScenePermutationAsset::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"scene", m_scene, AttributeType(type_of< SceneAsset >()));
	s >> MemberStlList< std::wstring >(L"includeLayers", m_includeLayers);

	if (s.getVersion() >= 1)
	{
		s >> MemberRef< world::WorldRenderSettings >(L"overrideWorldRenderSettings", m_overrideWorldRenderSettings);

		if (s.getVersion() >= 3)
			s >> MemberStaticArray< resource::Id< world::PostProcessSettings >, sizeof_array(m_overridePostProcessSettings), resource::Member< world::PostProcessSettings > >(L"overridePostProcessSettings", m_overridePostProcessSettings, c_PostProcessSettings_elementNames);
		else
		{
			resource::Id< world::PostProcessSettings > overridePostProcessSettings;
			s >> resource::Member< world::PostProcessSettings >(L"overridePostProcessSettings", overridePostProcessSettings);
			for (int32_t i = 0; i < sizeof_array(m_overridePostProcessSettings); ++i)
				m_overridePostProcessSettings[i] = overridePostProcessSettings;
		}
	}

	if (s.getVersion() >= 2)
	{
		s >> MemberSmallMap< std::wstring, resource::Id< render::ITexture >, Member< std::wstring >, resource::Member< render::ITexture > >(L"overridePostProcessParams", m_overridePostProcessParams);
	}
}

	}
}

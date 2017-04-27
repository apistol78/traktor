/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.scene.ScenePermutationAsset", 4, ScenePermutationAsset, ISerializable)

void ScenePermutationAsset::serialize(ISerializer& s)
{
	T_ASSERT (s.getVersion() >= 4);

	s >> Member< Guid >(L"scene", m_scene, AttributeType(type_of< SceneAsset >()));
	s >> MemberStlList< std::wstring >(L"includeLayers", m_includeLayers);
	s >> MemberRef< world::WorldRenderSettings >(L"overrideWorldRenderSettings", m_overrideWorldRenderSettings);
	s >> MemberSmallMap< std::wstring, resource::Id< render::ITexture >, Member< std::wstring >, resource::Member< render::ITexture > >(L"overrideImageProcessParams", m_overrideImageProcessParams);
}

	}
}

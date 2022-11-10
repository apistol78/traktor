/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.scene.ScenePermutationAsset", 5, ScenePermutationAsset, ISerializable)

void ScenePermutationAsset::serialize(ISerializer& s)
{
	T_ASSERT(s.getVersion() >= 4);

	s >> Member< Guid >(L"scene", m_scene, AttributeType(type_of< SceneAsset >()));
	s >> MemberStlList< std::wstring >(L"includeLayers", m_includeLayers);
	s >> MemberRef< world::WorldRenderSettings >(L"overrideWorldRenderSettings", m_overrideWorldRenderSettings);

	if (s.getVersion() < 5)
	{
		SmallMap< std::wstring, resource::Id< render::ITexture > > overrideImageProcessParams;
		s >> MemberSmallMap< std::wstring, resource::Id< render::ITexture >, Member< std::wstring >, resource::Member< render::ITexture > >(L"overrideImageProcessParams", overrideImageProcessParams);
	}
}

	}
}

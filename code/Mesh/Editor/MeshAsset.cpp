/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeNoHash.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Render/Editor/Texture/TextureSet.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.mesh.MeshAsset", 23, MeshAsset, editor::Asset)

void MeshAsset::serialize(ISerializer& s)
{
	const MemberEnum< MeshType >::Key c_MeshType_Keys[] =
	{
		{ L"MtInstance", MtInstance },
		{ L"MtSkinned", MtSkinned },
		{ L"MtStatic", MtStatic },

		// deprecated
		{ L"MtBlend", MtStatic },
		{ L"MtIndoor", MtStatic },
		{ L"MtLod", MtStatic },
		{ L"MtPartition", MtStatic },

		{ 0 }
	};

	editor::Asset::serialize(s);

	if (s.getVersion() >= 17)
		s >> Member< std::wstring >(L"importFilter", m_importFilter);

	s >> MemberEnum< MeshType >(L"meshType", m_meshType, c_MeshType_Keys);

	if (s.getVersion() >= 6 && s.getVersion() < 22)
	{
		SmallMap< std::wstring, Guid > materialTemplates;
		s >> MemberSmallMap< std::wstring, Guid >(L"materialTemplates", materialTemplates);
	}

	if (s.getVersion() >= 1)
		s >> MemberSmallMap< std::wstring, Guid >(L"materialShaders", m_materialShaders);

	if (s.getVersion() >= 4)
		s >> MemberSmallMap< std::wstring, Guid >(L"materialTextures", m_materialTextures);

	if (s.getVersion() >= 18 && s.getVersion() < 22)
		s >> ObsoleteMember< Guid >(L"textureSet", AttributeType(type_of< render::TextureSet >()));

	if (s.getVersion() >= 11)
		s >> Member< float >(L"scaleFactor", m_scaleFactor);

	if (s.getVersion() >= 19)
		s >> Member< Vector4 >(L"offset", m_offset);

	if (s.getVersion() >= 16)
		s >> Member< bool >(L"renormalize", m_renormalize);

	if (s.getVersion() >= 13)
		s >> Member< bool >(L"center", m_center);

	if (s.getVersion() >= 21)
		s >> Member< bool >(L"grounded", m_grounded);

	if (s.getVersion() >= 23)
		s >> Member< bool>(L"decalResponse", m_decalResponse);

	if (s.getVersion() >= 2 && s.getVersion() < 14)
		s >> ObsoleteMember< bool >(L"bakeOcclusion");
	
	if (s.getVersion() >= 3 && s.getVersion() < 15)
		s >> ObsoleteMember< bool >(L"cullDistantFaces");

	if (s.getVersion() < 12)
		s >> ObsoleteMember< bool >(L"generateOccluder");

	if (s.getVersion() >= 10)
	{
		s >> Member< int32_t >(L"lodSteps", m_lodSteps);
		s >> Member< float >(L"lodMaxDistance", m_lodMaxDistance);
		s >> Member< float >(L"lodCullDistance", m_lodCullDistance);
	}

	if (s.getVersion() < 12)
		s >> ObsoleteMember< Path >(L"occluderModel");

	if (s.getVersion() <= 8)
		s >> ObsoleteMember< float >(L"autoDetailLevel", AttributeRange(0.0f, 1.0f));

	if (s.getVersion() >= 20)
		s >> Member< float >(L"previewAngle", m_previewAngle, AttributeNoHash());
}

}

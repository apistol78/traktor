/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
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
#include "Core/Serialization/MemberStl.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Render/Editor/Texture/TextureSet.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.mesh.MeshAsset", 21, MeshAsset, editor::Asset)

void MeshAsset::serialize(ISerializer& s)
{
	const MemberEnum< MeshType >::Key c_MeshType_Keys[] =
	{
		{ L"MtBlend", MtBlend },
		{ L"MtIndoor", MtIndoor },
		{ L"MtInstance", MtInstance },
		{ L"MtLod", MtLod },
		{ L"MtPartition", MtPartition },
		{ L"MtSkinned", MtSkinned },
		{ L"MtStatic", MtStatic },
		{ 0 }
	};

	editor::Asset::serialize(s);

	if (s.getVersion() >= 17)
		s >> Member< std::wstring >(L"importFilter", m_importFilter);

	s >> MemberEnum< MeshType >(L"meshType", m_meshType, c_MeshType_Keys);

	if (s.getVersion() >= 6)
		s >> MemberStlMap< std::wstring, Guid >(L"materialTemplates", m_materialTemplates);

	if (s.getVersion() >= 1)
		s >> MemberStlMap< std::wstring, Guid >(L"materialShaders", m_materialShaders);

	if (s.getVersion() >= 4)
		s >> MemberStlMap< std::wstring, Guid >(L"materialTextures", m_materialTextures);

	if (s.getVersion() >= 18)
		s >> Member< Guid >(L"textureSet", m_textureSet, AttributeType(type_of< render::TextureSet >()));

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

	if (s.getVersion() >= 2 && s.getVersion() < 14)
	{
		bool bakeOcclusion = false;
		s >> Member< bool >(L"bakeOcclusion", bakeOcclusion);
	}
	
	if (s.getVersion() >= 3 && s.getVersion() < 15)
	{
		bool cullDistantFaces = false;
		s >> Member< bool >(L"cullDistantFaces", cullDistantFaces);
	}

	if (s.getVersion() < 12)
	{
		bool generateOccluder = false;
		s >> Member< bool >(L"generateOccluder", generateOccluder);
	}

	if (s.getVersion() >= 10)
	{
		s >> Member< int32_t >(L"lodSteps", m_lodSteps);
		s >> Member< float >(L"lodMaxDistance", m_lodMaxDistance);
		s >> Member< float >(L"lodCullDistance", m_lodCullDistance);
	}

	if (s.getVersion() < 12)
	{
		Path occluderModel;
		s >> Member< Path >(L"occluderModel", occluderModel);
	}

	if (s.getVersion() <= 8)
	{
		float autoDetailLevel = 0.0f;
		s >> Member< float >(L"autoDetailLevel", autoDetailLevel, AttributeRange(0.0f, 1.0f));
	}

	if (s.getVersion() >= 20)
		s >> Member< float >(L"previewAngle", m_previewAngle, AttributeNoHash());
}

	}
}

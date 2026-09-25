/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Texture/TrimSheetTextureAsset.h"

#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "Render/Editor/Texture/TrimSheetSetupAsset.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.TrimSheetTextureAsset", 0, TrimSheetTextureAsset, ISerializable)

void TrimSheetTextureAsset::serialize(ISerializer& s)
{
	const MemberEnum< TrimSheetLayer >::Key c_TrimSheetLayer_Keys[] = {
		{ L"Albedo", TrimSheetLayer::Albedo },
		{ L"Specular", TrimSheetLayer::Specular },
		{ L"Roughness", TrimSheetLayer::Roughness },
		{ L"Normal", TrimSheetLayer::Normal },
		{ L"Height", TrimSheetLayer::Height },
		{ 0 }
	};

	s >> Member< Guid >(L"setup", m_setup, AttributeType(type_of< TrimSheetSetupAsset >()));
	s >> MemberEnum< TrimSheetLayer >(L"layer", m_layer, c_TrimSheetLayer_Keys);
	s >> Member< bool >(L"keepAlpha", m_keepAlpha);
	s >> Member< bool >(L"generateMips", m_generateMips);
	s >> Member< bool >(L"enableCompression", m_enableCompression);
}

}

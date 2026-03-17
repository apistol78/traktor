/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Texture/ComputeTextureAsset.h"

#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "Render/Shader.h"
#include "Resource/Member.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.ComputeTextureAsset", 0, ComputeTextureAsset, ISerializable)

void ComputeTextureAsset::serialize(ISerializer& s)
{
	s >> resource::Member< Shader >(L"shader", m_shader);
	s >> Member< int32_t >(L"width", m_width);
	s >> Member< int32_t >(L"height", m_height);

	const MemberEnum< TextureFormat >::Key c_TextureFormat_Keys[] =
	{
		{ L"TfR8", TfR8 },
		{ L"TfR8G8B8A8", TfR8G8B8A8 },
		{ L"TfR16G16B16A16F", TfR16G16B16A16F },
		{ L"TfR32G32B32A32F", TfR32G32B32A32F },
		{ L"TfR16F", TfR16F },
		{ L"TfR32F", TfR32F },
		{ L"TfR16G16F", TfR16G16F },
		{ L"TfR32G32F", TfR32G32F },
		{ L"TfR11G11B10F", TfR11G11B10F },
		{ 0 }
	};
	s >> MemberEnum< TextureFormat >(L"format", m_format, c_TextureFormat_Keys);

	s >> Member< bool >(L"continuous", m_continuous);
}

}

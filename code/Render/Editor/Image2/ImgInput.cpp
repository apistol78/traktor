/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Render/Editor/Image2/ImgInput.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const ImmutableNode::OutputPinDesc c_ImgInput_o[] = { { L"Output" }, { 0 } };

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImgInput", 0, ImgInput, ImmutableNode)

ImgInput::ImgInput()
:	ImmutableNode(nullptr, c_ImgInput_o)
{
}

const std::wstring& ImgInput::getTextureId() const
{
	return m_textureId;
}

void ImgInput::serialize(ISerializer& s)
{
	Node::serialize(s);
	s >> Member< std::wstring >(L"textureId", m_textureId);
}

	}
}
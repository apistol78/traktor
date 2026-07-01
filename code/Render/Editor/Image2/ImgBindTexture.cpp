/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Render/ITexture.h"
#include "Render/Editor/Image2/ImgBindTexture.h"
#include "Resource/Member.h"

namespace traktor::render
{
	namespace
	{

const ImmutableNode::OutputPinDesc c_ImgBindTexture_o[] = { { L"Output", L"{42CDF5D7-B6EF-43C2-B889-0D5621AB4DFF}" }, { 0 } };

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImgBindTexture", 0, ImgBindTexture, ImmutableNode)

ImgBindTexture::ImgBindTexture()
:	ImmutableNode(nullptr, c_ImgBindTexture_o)
{
}

const resource::Id< ITexture >& ImgBindTexture::getTexture() const
{
	return m_texture;
}

void ImgBindTexture::serialize(ISerializer& s)
{
	Node::serialize(s);
	s >> resource::Member< ITexture >(L"texture", m_texture);
}

}

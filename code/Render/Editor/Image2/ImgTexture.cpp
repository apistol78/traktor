/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Render/ITexture.h"
#include "Render/Editor/Image2/ImgTexture.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const ImmutableNode::OutputPinDesc c_ImgTexture_o[] = { { L"Output" }, { 0 } };

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImgTexture", 0, ImgTexture, ImmutableNode)

ImgTexture::ImgTexture()
:	ImmutableNode(nullptr, c_ImgTexture_o)
{
}

const resource::Id< ITexture >& ImgTexture::getTexture() const
{
	return m_texture;
}

void ImgTexture::serialize(ISerializer& s)
{
	Node::serialize(s);
	s >> resource::Member< ITexture >(L"texture", m_texture);
}

	}
}
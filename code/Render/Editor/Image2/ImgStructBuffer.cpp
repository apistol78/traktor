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
#include "Render/Editor/Image2/ImgStructBuffer.h"

namespace traktor::render
{
	namespace
	{

const ImmutableNode::InputPinDesc c_ImgStructBuffer_i[] = { { L"Input", L"{96622082-61C1-4849-95C6-44E30EACB145}" }, { 0 } };
const ImmutableNode::OutputPinDesc c_ImgStructBuffer_o[] = { { L"Output", L"{63CE19D8-A53D-48B5-9931-3BCC4B5807AF}" }, { 0 } };

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImgStructBuffer", 0, ImgStructBuffer, ImmutableNode)

ImgStructBuffer::ImgStructBuffer()
:	ImmutableNode(c_ImgStructBuffer_i, c_ImgStructBuffer_o)
{
}

bool ImgStructBuffer::getPersistent() const
{
	return m_persistent;
}

void ImgStructBuffer::serialize(ISerializer& s)
{
	Node::serialize(s);

	s >> Member< bool >(L"persistent", m_persistent);
	s >> Member< uint32_t >(L"elementCount", m_elementCount);
	s >> Member< uint32_t >(L"elementSize", m_elementSize);
}

}

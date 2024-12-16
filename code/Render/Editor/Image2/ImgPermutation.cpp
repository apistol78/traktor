/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Render/Editor/Image2/ImgPermutation.h"

namespace traktor::render
{
	namespace
	{

const ImmutableNode::InputPinDesc c_ImgPermutation_i[] = { { L"True", L"{71E4277E-8919-4FC6-B289-532EF34E0702}", false }, { L"False", L"{21DBF30F-4888-47EA-943C-AC8ABF38E8DF}", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_ImgPermutation_o[] = { { L"Output", L"{9B51E249-A97B-40E3-9CD0-FA2C714FE243}" }, { 0 } };

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImgPermutation", 0, ImgPermutation, ImmutableNode)

ImgPermutation::ImgPermutation()
:	ImmutableNode(c_ImgPermutation_i, c_ImgPermutation_o)
{
}

void ImgPermutation::serialize(ISerializer& s)
{
	Node::serialize(s);
	s >> Member< std::wstring >(L"name", m_name);
}

}

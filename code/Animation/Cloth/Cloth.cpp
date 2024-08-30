/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Cloth/Cloth.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStaticArray.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.Cloth", 0, Cloth, ISerializable)

void Cloth::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< Node, MemberComposite< Node > >(L"nodes", m_nodes);
	s >> MemberAlignedVector< Edge, MemberComposite< Edge > >(L"edges", m_edges);
	s >> MemberAlignedVector< int32_t >(L"triangles", m_triangles);
}

void Cloth::Node::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"position", position);
	s >> Member< Vector2 >(L"texCoord", texCoord);
	s >> Member< float >(L"invMass", invMass);
	s >> Member< int32_t >(L"east", east);
	s >> Member< int32_t >(L"north", north);
}

void Cloth::Edge::serialize(ISerializer& s)
{
	s >> MemberStaticArray< int32_t, 2 >(L"indices", indices);
	s >> Member< float >(L"length", length);
}

}

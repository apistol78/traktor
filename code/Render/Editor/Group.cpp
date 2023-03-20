/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeMultiLine.h"
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Render/Editor/Group.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Group", 0, Group, ISerializable)

const std::wstring& Group::getTitle() const
{
	return m_title;
}

void Group::setPosition(const std::pair< int, int >& position)
{
	m_position = position;
}

const std::pair< int, int >& Group::getPosition() const
{
	return m_position;
}

void Group::setSize(const std::pair< int, int >& size)
{
	m_size = size;
}

const std::pair< int, int >& Group::getSize() const
{
	return m_size;
}

void Group::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"title", m_title);
	s >> Member< std::wstring >(L"comment", m_comment, AttributeMultiLine());
	s >> MemberStlPair< int32_t, int32_t >(L"position", m_position, AttributePrivate());
	s >> MemberStlPair< int32_t, int32_t >(L"size", m_size, AttributePrivate());
}

}

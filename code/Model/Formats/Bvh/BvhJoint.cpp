/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Model/Formats/Bvh/BvhJoint.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.BvhJoint", BvhJoint, Object)

BvhJoint::BvhJoint(const std::wstring& name)
:	m_name(name)
,	m_offset(0.0f, 0.0f, 0.0f, 0.0f)
,	m_channelOffset(0)
,	m_parent(nullptr)
{
}

const std::wstring& BvhJoint::getName() const
{
	return m_name;
}

void BvhJoint::setOffset(const Vector4& offset)
{
	m_offset = offset;
}

const Vector4& BvhJoint::getOffset() const
{
	return m_offset;
}

void BvhJoint::setChannelOffset(int32_t channelOffset)
{
	m_channelOffset = channelOffset;
}

int32_t BvhJoint::getChannelOffset() const
{
	return m_channelOffset;
}

void BvhJoint::addChannel(const std::wstring& channelName)
{
	m_channels.push_back(channelName);
}

const std::vector< std::wstring >& BvhJoint::getChannels() const
{
	return m_channels;
}

void BvhJoint::addChild(BvhJoint* child)
{
	T_ASSERT (child);
	child->m_parent = this;
	m_children.push_back(child);
}

BvhJoint* BvhJoint::getParent() const
{
	return m_parent;
}

const RefArray< BvhJoint >& BvhJoint::getChildren() const
{
	return m_children;
}

	}
}

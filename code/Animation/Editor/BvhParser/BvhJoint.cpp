/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Animation/Editor/BvhParser/BvhJoint.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.BvhJoint", BvhJoint, Object)

BvhJoint::BvhJoint(const std::wstring& name)
:	m_name(name)
,	m_offset(0.0f, 0.0f, 0.0f, 0.0f)
,	m_channelOffset(0)
,	m_parent(0)
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

/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include <vector>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Math/Vector4.h"

namespace traktor::model
{

/*!
 * \ingroup Model
 */
class BvhJoint : public Object
{
	T_RTTI_CLASS;

public:
	explicit BvhJoint(const std::wstring& name);

	const std::wstring& getName() const;

	void setOffset(const Vector4& offset);

	const Vector4& getOffset() const;

	void setChannelOffset(int32_t channelOffset);

	int32_t getChannelOffset() const;

	void addChannel(const std::wstring& channelName);

	const std::vector< std::wstring >& getChannels() const;

	void addChild(BvhJoint* child);

	BvhJoint* getParent() const;

	const RefArray< BvhJoint >& getChildren() const;

private:
	std::wstring m_name;
	Vector4 m_offset;
	int32_t m_channelOffset;
	std::vector< std::wstring > m_channels;
	BvhJoint* m_parent;
	RefArray< BvhJoint > m_children;
};

}

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_animation_BvhJoint_H
#define traktor_animation_BvhJoint_H

#include <string>
#include <vector>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Math/Vector4.h"

namespace traktor
{
	namespace animation
	{

/*! \brief
 * \ingroup Animation
 */
class BvhJoint : public Object
{
	T_RTTI_CLASS;

public:
	BvhJoint(const std::wstring& name);

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
}

#endif	// traktor_animation_BvhJoint_H

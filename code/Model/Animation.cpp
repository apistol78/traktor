/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Model/Animation.h"
#include "Model/Pose.h"

namespace traktor::model
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.model.Animation", 0, Animation, ISerializable)

void Animation::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& Animation::getName() const
{
	return m_name;
}

void Animation::insertKeyFrame(float time, const Pose* pose)
{
	KeyFrame kf;
	kf.time = time;
	kf.pose = pose;
	m_keyFrames.push_back(kf);
}

uint32_t Animation::getKeyFrameCount() const
{
	return (uint32_t)m_keyFrames.size();
}

float Animation::getKeyFrameTime(uint32_t keyFrame) const
{
	return m_keyFrames[keyFrame].time;
}

void Animation::setKeyFramePose(uint32_t keyFrame, const Pose* pose)
{
	m_keyFrames[keyFrame].pose = pose;
}

const Pose* Animation::getKeyFramePose(uint32_t keyFrame) const
{
	return m_keyFrames[keyFrame].pose;
}

void Animation::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> MemberAlignedVector< KeyFrame, MemberComposite< KeyFrame > >(L"keyFrames", m_keyFrames);
}

void Animation::KeyFrame::serialize(ISerializer& s)
{
	s >> Member< float >(L"time", time);
	s >> MemberRef< const Pose >(L"pose", pose);
}

}

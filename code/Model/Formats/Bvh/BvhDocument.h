/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include <vector>
#include "Core/Object.h"
#include "Core/Ref.h"

namespace traktor
{

class IStream;
class Vector4;

}

namespace traktor::model
{

class BvhJoint;

/*! \brief
 * \ingroup Model
 */
class BvhDocument : public Object
{
	T_RTTI_CLASS;

public:
	typedef std::vector< float > cv_t;
	typedef std::list< cv_t > cv_list_t;

	static Ref< BvhDocument > parse(IStream* stream);

	BvhJoint* getRootJoint() const;

	float getFrameTime() const;

	const cv_list_t& getChannelValues() const;

private:
	Ref< BvhJoint > m_rootJoint;
	float m_frameTime;
	cv_list_t m_channelValues;
};

}

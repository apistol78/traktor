/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Containers/AlignedVector.h"
#include "Core/Ref.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor
{

class ISerializer;

}

namespace traktor::forum
{

/*! Forum post.
 * \ingroup Webber
 */
struct Post
{
	std::wstring author;
	std::wstring text;
	uint64_t time;	//!< Seconds since the Epoch.
	int32_t id = 0;	//!< Unique within the forum; never reused.

	void serialize(ISerializer& s);
};

/*! Forum thread; removed together with its last post.
 * \ingroup Webber
 */
struct Thread
{
	std::wstring title;
	AlignedVector< Post > posts;
	int32_t id = 0;	//!< Unique within the forum; never reused.

	void serialize(ISerializer& s);
};

/*! Forum board.
 * \ingroup Webber
 */
struct Board
{
	std::wstring name;
	std::wstring description;
	AlignedVector< Thread > threads;

	void serialize(ISerializer& s);
};

/*! In-memory forum, shared by all sessions; root of the forum data file.
 * \ingroup Webber
 */
class Forum : public ISerializable
{
	T_RTTI_CLASS;

public:
	int32_t nextId = 1;	//!< Id of next new thread or post.
	std::wstring name;
	AlignedVector< Board > boards;

	virtual void serialize(ISerializer& s) override final;
};

/*! Create a forum populated with sample boards, threads and posts. */
Ref< Forum > createSampleForum();

}

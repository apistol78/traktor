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
};

/*! Forum thread; always has at least one post.
 * \ingroup Webber
 */
struct Thread
{
	std::wstring title;
	AlignedVector< Post > posts;
};

/*! Forum board.
 * \ingroup Webber
 */
struct Board
{
	std::wstring name;
	std::wstring description;
	AlignedVector< Thread > threads;
};

/*! In-memory forum, shared by all sessions.
 * \ingroup Webber
 */
struct Forum
{
	std::wstring name;
	AlignedVector< Board > boards;
};

/*! Create a forum populated with sample boards, threads and posts. */
Forum createSampleForum();

}

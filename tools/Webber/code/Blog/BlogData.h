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

namespace traktor::blog
{

/*! Blog post.
 * \ingroup Webber
 */
struct Post
{
	std::wstring title;
	std::wstring author;
	uint64_t time;	//!< Seconds since the Epoch.
	AlignedVector< std::wstring > paragraphs;
};

/*! In-memory blog, shared by all sessions.
 * \ingroup Webber
 */
struct Blog
{
	AlignedVector< Post > posts;
};

/*! Create a blog populated with sample posts. */
Blog createSampleBlog();

}

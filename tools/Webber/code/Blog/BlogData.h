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
	std::wstring text;	//!< Markdown.
	int32_t id = 0;	//!< Unique within the blog; never reused.

	void serialize(ISerializer& s);
};

/*! In-memory blog, shared by all sessions; root of the blog data file.
 * \ingroup Webber
 */
class Blog : public ISerializable
{
	T_RTTI_CLASS;

public:
	int32_t nextId = 1;	//!< Id of next new post.
	AlignedVector< Post > posts;

	virtual void serialize(ISerializer& s) override final;
};

/*! Create a blog populated with sample posts. */
Ref< Blog > createSampleBlog();

}

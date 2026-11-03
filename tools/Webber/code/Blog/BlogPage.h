/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Webber/Container.h"

namespace traktor::blog
{

struct Blog;

/*! Blog reader page; lists posts and shows a single post.
 * \ingroup Webber
 */
class BlogPage : public wbr::Container
{
	T_RTTI_CLASS;

public:
	explicit BlogPage(const Blog& blog);

	void showPosts();

private:
	const Blog& m_blog;

	void showPost(int32_t post);
};

}

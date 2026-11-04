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

}

namespace traktor::forum
{

struct Forum;

}

namespace traktor::demo
{

class DemoApplication;

/*! Landing page; introduction, latest blog posts and forum threads.
 * \ingroup Webber
 */
class HomePage : public wbr::Container
{
	T_RTTI_CLASS;

public:
	/*! \param app Application which navigates to pages linked from here; must outlive the page. */
	explicit HomePage(DemoApplication* app, const blog::Blog& blog, const forum::Forum& forum);
};

}

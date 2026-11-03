/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Webber/IApplication.h"

namespace traktor::wbr
{

class Container;
class Link;

}

namespace traktor::blog
{

struct Blog;
class BlogPage;

}

namespace traktor::forum
{

struct Forum;
class ForumPage;

}

namespace traktor::demo
{

/*! Demo site; top bar with links to the blog and the forum.
 * \ingroup Webber
 */
class DemoApplication : public wbr::IApplication
{
	T_RTTI_CLASS;

public:
	explicit DemoApplication(const blog::Blog& blog, forum::Forum& forum);

	virtual bool create() override final;

	virtual void destroy() override final;

	virtual wbr::Widget* getRootWidget() const override final;

	virtual std::wstring getStyleSheet() const override final;

	virtual Ref< IStream > resolve(const std::wstring& uri) const override final;

private:
	const blog::Blog& m_blogData;
	forum::Forum& m_forumData;
	Ref< wbr::Container > m_root;
	Ref< wbr::Link > m_blogLink;
	Ref< wbr::Link > m_forumLink;
	Ref< wbr::Container > m_content;
	Ref< blog::BlogPage > m_blog;
	Ref< forum::ForumPage > m_forum;

	void showBlog();

	void showForum();
};

}

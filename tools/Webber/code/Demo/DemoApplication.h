/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Webber/IApplication.h"

namespace traktor::wbr
{

class Container;
class Link;
class Widget;

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

struct User;

/*! Demo site; top bar with links to home, blog and forum, and login.
 * \ingroup Webber
 */
class DemoApplication : public wbr::IApplication
{
	T_RTTI_CLASS;

public:
	explicit DemoApplication(const AlignedVector< User >& users, blog::Blog& blog, forum::Forum& forum);

	virtual bool create() override final;

	virtual void destroy() override final;

	virtual wbr::Widget* getRootWidget() const override final;

	virtual std::wstring getStyleSheet() const override final;

	virtual Ref< IStream > resolve(const std::wstring& uri) const override final;

	virtual std::wstring getLocation() const override final;

	virtual void navigate(const std::wstring& location) override final;

	void showHome();

	void showBlog();

	void showBlogPost(int32_t post);

	void showForum();

	void showForumThread(int32_t board, int32_t thread);

private:
	const AlignedVector< User >& m_users;
	blog::Blog& m_blogData;
	forum::Forum& m_forumData;
	Ref< wbr::Container > m_root;
	Ref< wbr::Link > m_homeLink;
	Ref< wbr::Link > m_blogLink;
	Ref< wbr::Link > m_forumLink;
	wbr::Link* m_activeLink = nullptr;	//!< Link of the page being shown.
	Ref< wbr::Container > m_userInfo;
	Ref< wbr::Container > m_content;
	Ref< wbr::Container > m_overlay;
	Ref< blog::BlogPage > m_blog;
	Ref< forum::ForumPage > m_forum;

	/*! Replace content with page and highlight its link in the top bar. */
	void showPage(wbr::Widget* page, wbr::Link* link);

	/*! Show login dialog on top of current page. */
	void showLogin();

	/*! \param user Logged in user, null for guest. */
	void setUser(const User* user);
};

}

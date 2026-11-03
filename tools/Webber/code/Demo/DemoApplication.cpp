/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/MemoryStream.h"
#include "Blog/BlogPage.h"
#include "Demo/DemoApplication.h"
#include "Forum/ForumPage.h"
#include "Webber/Container.h"
#include "Webber/Image.h"
#include "Webber/Link.h"
#include "Webber/Static.h"

// Resources
#include "Resources/Demo.h"
#include "Resources/Logo.h"

namespace traktor::demo
{
	namespace
	{

const wchar_t* c_user = L"Guest";

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.demo.DemoApplication", DemoApplication, wbr::IApplication)

DemoApplication::DemoApplication(const blog::Blog& blog, forum::Forum& forum)
:	m_blogData(blog)
,	m_forumData(forum)
{
}

bool DemoApplication::create()
{
	m_root = new wbr::Container();
	m_root->setStyleClass(L"demo_page");

	auto topBar = m_root->addChild(new wbr::Container());
	topBar->setStyleClass(L"demo_topbar");
	topBar->addChild(new wbr::Image(L"Logo.webp", L"Webber Forum"))->setStyleClass(L"demo_logo");

	auto links = topBar->addChild(new wbr::Container());
	links->setStyleClass(L"demo_links");
	m_blogLink = links->addChild(new wbr::Link(L"Blog"));
	m_blogLink->clicked() += [this]() { showBlog(); };
	m_forumLink = links->addChild(new wbr::Link(L"Forum"));
	m_forumLink->clicked() += [this]() { showForum(); };

	topBar->addChild(new wbr::Static(std::wstring(L"Logged in as ") + c_user))->setStyleClass(L"demo_user");

	m_content = m_root->addChild(new wbr::Container());
	m_content->setStyleClass(L"demo_content");

	m_blog = new blog::BlogPage(m_blogData);
	m_forum = new forum::ForumPage(m_forumData, c_user);

	showBlog();
	return true;
}

void DemoApplication::destroy()
{
	m_forum = nullptr;
	m_blog = nullptr;
	m_content = nullptr;
	m_forumLink = nullptr;
	m_blogLink = nullptr;
	m_root = nullptr;
}

wbr::Widget* DemoApplication::getRootWidget() const
{
	return m_root;
}

std::wstring DemoApplication::getStyleSheet() const
{
	return L"Demo.css";
}

Ref< IStream > DemoApplication::resolve(const std::wstring& uri) const
{
	if (uri == L"/Demo.css")
		return new MemoryStream(c_ResourceDemo, sizeof(c_ResourceDemo));
	else if (uri == L"/Logo.webp")
		return new MemoryStream(c_ResourceLogo, sizeof(c_ResourceLogo));
	else
		return nullptr;
}

void DemoApplication::showBlog()
{
	m_blogLink->setStyleClass(L"demo_active");
	m_forumLink->setStyleClass(L"");
	m_content->removeAllChildren();
	m_content->addChild(m_blog.ptr());
	m_blog->showPosts();
}

void DemoApplication::showForum()
{
	m_blogLink->setStyleClass(L"");
	m_forumLink->setStyleClass(L"demo_active");
	m_content->removeAllChildren();
	m_content->addChild(m_forum.ptr());
	m_forum->showBoards();
}

}

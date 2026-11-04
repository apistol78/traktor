/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/MemoryStream.h"
#include "Core/Misc/String.h"
#include "Blog/BlogPage.h"
#include "Demo/DemoApplication.h"
#include "Demo/HomePage.h"
#include "Demo/UserData.h"
#include "Forum/ForumPage.h"
#include "Webber/Button.h"
#include "Webber/Container.h"
#include "Webber/Edit.h"
#include "Webber/Link.h"
#include "Webber/Static.h"

// Resources
#include "Resources/Demo.h"

namespace traktor::demo
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.demo.DemoApplication", DemoApplication, wbr::IApplication)

DemoApplication::DemoApplication(
	const AlignedVector< User >& users,
	blog::Blog& blog,
	const std::function< void() >& blogChanged,
	forum::Forum& forum,
	const std::function< void() >& forumChanged
)
:	m_users(users)
,	m_blogData(blog)
,	m_blogChanged(blogChanged)
,	m_forumData(forum)
,	m_forumChanged(forumChanged)
{
}

bool DemoApplication::create()
{
	m_root = new wbr::Container();
	m_root->setStyleClass(L"demo_page");

	auto topBar = m_root->addChild(new wbr::Container());
	topBar->setStyleClass(L"demo_topbar");
	auto bar = topBar->addChild(new wbr::Container());
	bar->setStyleClass(L"demo_topbar_inner");

	auto logo = bar->addChild(new wbr::Container());
	logo->setStyleClass(L"demo_logo");
	logo->addChild(new wbr::Static(L"{ }"))->setStyleClass(L"demo_logo_mark");
	logo->addChild(new wbr::Static(L"Veterankod"))->setStyleClass(L"demo_logo_name");

	auto links = bar->addChild(new wbr::Container());
	links->setStyleClass(L"demo_links");
	m_homeLink = links->addChild(new wbr::Link(L"Home"));
	m_homeLink->clicked() += [this]() { showHome(); };
	m_blogLink = links->addChild(new wbr::Link(L"Blog"));
	m_blogLink->clicked() += [this]() { showBlog(); };
	m_forumLink = links->addChild(new wbr::Link(L"Forum"));
	m_forumLink->clicked() += [this]() { showForum(); };

	m_userInfo = bar->addChild(new wbr::Container());
	m_userInfo->setStyleClass(L"demo_user");

	m_content = m_root->addChild(new wbr::Container());
	m_content->setStyleClass(L"demo_content");

	// Holds the login dialog; empty when no dialog is shown.
	m_overlay = m_root->addChild(new wbr::Container());

	m_blog = new blog::BlogPage(m_blogData, m_blogChanged);
	m_forum = new forum::ForumPage(m_forumData, m_forumChanged);

	setUser(nullptr);
	showHome();
	return true;
}

void DemoApplication::destroy()
{
	m_forum = nullptr;
	m_blog = nullptr;
	m_overlay = nullptr;
	m_content = nullptr;
	m_userInfo = nullptr;
	m_forumLink = nullptr;
	m_blogLink = nullptr;
	m_homeLink = nullptr;
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
	else
		return nullptr;
}

std::wstring DemoApplication::getLocation() const
{
	if (m_activeLink == m_blogLink)
	{
		const std::wstring location = m_blog->getLocation();
		return location.empty() ? L"blog" : L"blog/" + location;
	}
	else if (m_activeLink == m_forumLink)
	{
		const std::wstring location = m_forum->getLocation();
		return location.empty() ? L"forum" : L"forum/" + location;
	}
	else
		return L"";
}

void DemoApplication::navigate(const std::wstring& location)
{
	m_overlay->removeAllChildren();

	const size_t p = location.find(L'/');
	const std::wstring section = location.substr(0, p);
	const std::wstring rest = (p != location.npos) ? location.substr(p + 1) : L"";

	if (section == L"blog")
	{
		showPage(m_blog, m_blogLink);
		m_blog->navigate(rest);
	}
	else if (section == L"forum")
	{
		showPage(m_forum, m_forumLink);
		m_forum->navigate(rest);
	}
	else
		showHome();
}

void DemoApplication::showHome()
{
	showPage(new HomePage(this, m_blogData, m_forumData), m_homeLink);
}

void DemoApplication::showBlog()
{
	showPage(m_blog, m_blogLink);
	m_blog->showPosts();
}

void DemoApplication::showBlogPost(int32_t post)
{
	showPage(m_blog, m_blogLink);
	m_blog->showPost(post);
}

void DemoApplication::showForum()
{
	showPage(m_forum, m_forumLink);
	m_forum->showBoards();
}

void DemoApplication::showForumThread(int32_t board, int32_t thread)
{
	showPage(m_forum, m_forumLink);
	m_forum->showThread(board, thread);
}

void DemoApplication::showPage(wbr::Widget* page, wbr::Link* link)
{
	m_activeLink = link;
	m_homeLink->setStyleClass(link == m_homeLink ? L"demo_active" : L"");
	m_blogLink->setStyleClass(link == m_blogLink ? L"demo_active" : L"");
	m_forumLink->setStyleClass(link == m_forumLink ? L"demo_active" : L"");
	m_content->removeAllChildren();
	m_content->addChild(page);
}

void DemoApplication::showLogin()
{
	m_overlay->removeAllChildren();

	auto backdrop = m_overlay->addChild(new wbr::Container());
	backdrop->setStyleClass(L"demo_backdrop");

	auto form = backdrop->addChild(new wbr::Container());
	form->setStyleClass(L"demo_login");
	form->addChild(new wbr::Static(L"Log in"))->setStyleClass(L"demo_heading");
	form->addChild(new wbr::Static(L"User name"))->setStyleClass(L"demo_label");
	auto name = form->addChild(new wbr::Edit());
	form->addChild(new wbr::Static(L"Password"))->setStyleClass(L"demo_label");
	auto password = form->addChild(new wbr::Edit());
	password->setPassword(true);
	auto error = form->addChild(new wbr::Static(L""));
	error->setStyleClass(L"demo_error");

	auto buttons = form->addChild(new wbr::Container());
	buttons->addChild(new wbr::Button(L"Log in"))->clicked() += [this, name, password, error]() {
		for (const auto& user : m_users)
		{
			if (user.name == trim(name->getText()) && user.password == password->getText())
			{
				m_overlay->removeAllChildren();
				setUser(&user);
				return;
			}
		}
		error->setText(L"Unknown user name or wrong password.");
	};
	buttons->addChild(new wbr::Button(L"Cancel"))->clicked() += [this]() { m_overlay->removeAllChildren(); };
}

void DemoApplication::setUser(const User* user)
{
	m_blog->setUser(user ? user->name : L"", user && user->admin);
	m_forum->setUser(user ? user->name : L"", user && user->admin);

	m_userInfo->removeAllChildren();
	if (user)
	{
		m_userInfo->addChild(new wbr::Static(user->name + (user->admin ? L" (admin)" : L"")));
		m_userInfo->addChild(new wbr::Button(L"Log out"))->clicked() += [this]() { setUser(nullptr); };
	}
	else
	{
		m_userInfo->addChild(new wbr::Static(L"Guest"));
		m_userInfo->addChild(new wbr::Button(L"Log in"))->clicked() += [this]() { showLogin(); };
	}
}

}

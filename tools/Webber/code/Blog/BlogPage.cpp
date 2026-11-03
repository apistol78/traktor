/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <vector>
#include "Core/Date/DateTime.h"
#include "Blog/BlogData.h"
#include "Blog/BlogPage.h"
#include "Webber/Link.h"
#include "Webber/Static.h"

namespace traktor::blog
{
	namespace
	{

std::wstring formatByline(const Post& post)
{
	return DateTime(post.time).format(L"%Y-%m-%d") + L" · " + post.author;
}

wbr::Container* addContainer(wbr::Container* parent, const std::wstring& styleClass)
{
	auto container = parent->addChild(new wbr::Container());
	container->setStyleClass(styleClass);
	return container;
}

wbr::Static* addStatic(wbr::Container* parent, const std::wstring& text, const std::wstring& styleClass)
{
	auto st = parent->addChild(new wbr::Static(text));
	st->setStyleClass(styleClass);
	return st;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.blog.BlogPage", BlogPage, wbr::Container)

BlogPage::BlogPage(const Blog& blog)
:	m_blog(blog)
{
	setStyleClass(L"blog");
	showPosts();
}

void BlogPage::showPosts()
{
	removeAllChildren();
	addStatic(this, L"Latest posts", L"blog_heading");

	// Newest first.
	std::vector< int32_t > order(m_blog.posts.size());
	for (int32_t i = 0; i < (int32_t)order.size(); ++i)
		order[i] = i;
	std::stable_sort(order.begin(), order.end(), [&](int32_t lh, int32_t rh) {
		return m_blog.posts[lh].time > m_blog.posts[rh].time;
	});

	for (auto i : order)
	{
		const Post& post = m_blog.posts[i];
		auto summary = addContainer(this, L"blog_summary");
		auto title = summary->addChild(new wbr::Link(post.title));
		title->setStyleClass(L"blog_summary_title");
		title->clicked() += [this, i]() { showPost(i); };
		addStatic(summary, formatByline(post), L"blog_byline");
		if (!post.paragraphs.empty())
			addStatic(summary, post.paragraphs.front(), L"blog_excerpt");
	}
}

void BlogPage::showPost(int32_t post)
{
	const Post& p = m_blog.posts[post];

	removeAllChildren();

	auto back = addChild(new wbr::Link(L"‹ All posts"));
	back->setStyleClass(L"blog_back");
	back->clicked() += [this]() { showPosts(); };

	auto article = addContainer(this, L"blog_article");
	addStatic(article, p.title, L"blog_title");
	addStatic(article, formatByline(p), L"blog_byline");
	for (const auto& paragraph : p.paragraphs)
		addStatic(article, paragraph, L"blog_paragraph");
}

}

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
#include "Core/Misc/String.h"
#include "Blog/BlogData.h"
#include "Blog/BlogPage.h"
#include "Webber/Button.h"
#include "Webber/Edit.h"
#include "Webber/FileUpload.h"
#include "Webber/Link.h"
#include "Webber/Markdown.h"
#include "Webber/Static.h"
#include "Webber/TextArea.h"

namespace traktor::blog
{
	namespace
	{

/*! Post id from location segment, -1 unless only digits. */
int32_t parseId(const std::wstring& text)
{
	if (text.empty() || text.size() > 9 || text.find_first_not_of(L"0123456789") != text.npos)
		return -1;
	return parseString< int32_t >(text);
}

Post* findPost(Blog& blog, int32_t id)
{
	auto it = std::find_if(blog.posts.begin(), blog.posts.end(), [=](const Post& post) { return post.id == id; });
	return it != blog.posts.end() ? &(*it) : nullptr;
}

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

BlogPage::BlogPage(Blog& blog, const std::function< void() >& changed, const std::function< std::wstring(const AlignedVector< uint8_t >&) >& storeImage)
:	m_blog(blog)
,	m_changed(changed)
,	m_storeImage(storeImage)
{
	setStyleClass(L"blog");
	showPosts();
}

void BlogPage::setUser(const std::wstring& user, bool admin)
{
	m_user = user;
	m_admin = admin;
	if (m_post >= 0)
		showPost(m_post);
	else
		showPosts();
}

std::wstring BlogPage::getLocation() const
{
	if (m_editing)
		return m_post >= 0 ? toString(m_post) + L"/edit" : L"new";
	else
		return m_post >= 0 ? toString(m_post) : L"";
}

void BlogPage::navigate(const std::wstring& location)
{
	const size_t p = location.find(L'/');
	const std::wstring first = location.substr(0, p);
	const std::wstring rest = (p != location.npos) ? location.substr(p + 1) : L"";
	const int32_t post = parseId(first);

	if (first == L"new" && m_admin)
		showEditor(-1);
	else if (!findPost(m_blog, post))
		showPosts();
	else if (rest == L"edit" && m_admin)
		showEditor(post);
	else
		showPost(post);
}

void BlogPage::showPosts()
{
	m_post = -1;
	m_editing = false;
	removeAllChildren();
	addStatic(this, L"Latest posts", L"blog_heading");
	if (m_admin)
		addChild(new wbr::Button(L"New post"))->clicked() += [this]() { showEditor(-1); };

	// Newest first.
	std::vector< const Post* > order;
	for (const auto& post : m_blog.posts)
		order.push_back(&post);
	std::stable_sort(order.begin(), order.end(), [](const Post* lh, const Post* rh) {
		return lh->time > rh->time;
	});

	for (const Post* post : order)
	{
		const int32_t id = post->id;
		auto summary = addContainer(this, L"blog_summary");
		auto title = summary->addChild(new wbr::Link(post->title));
		title->setStyleClass(L"blog_summary_title");
		title->clicked() += [this, id]() { showPost(id); };
		addStatic(summary, formatByline(*post), L"blog_byline");

		// Excerpt is the first block, up to the first blank line.
		summary->addChild(new wbr::Markdown(post->text.substr(0, post->text.find(L"\n\n"))))->setStyleClass(L"blog_excerpt");
	}
}

void BlogPage::showPost(int32_t post)
{
	const Post* p = findPost(m_blog, post);
	if (!p)
	{
		showPosts();
		return;
	}

	m_post = post;
	m_editing = false;
	removeAllChildren();

	auto back = addChild(new wbr::Link(L"‹ All posts"));
	back->setStyleClass(L"blog_back");
	back->clicked() += [this]() { showPosts(); };

	auto article = addContainer(this, L"blog_article");
	addStatic(article, p->title, L"blog_title");
	addStatic(article, formatByline(*p), L"blog_byline");
	article->addChild(new wbr::Markdown(p->text))->setStyleClass(L"blog_text");

	if (m_admin)
	{
		article->addChild(new wbr::Button(L"Edit"))->clicked() += [this, post]() { showEditor(post); };
		article->addChild(new wbr::Button(L"Delete"))->clicked() += [this, post]() {
			auto& posts = m_blog.posts;
			posts.erase(std::remove_if(posts.begin(), posts.end(), [=](const Post& p) { return p.id == post; }), posts.end());
			m_changed();
			showPosts();
		};
	}
}

void BlogPage::showEditor(int32_t post)
{
	const Post* p = post >= 0 ? findPost(m_blog, post) : nullptr;
	if (post >= 0 && !p)
	{
		showPosts();
		return;
	}

	m_post = post;
	m_editing = true;
	removeAllChildren();
	addStatic(this, post >= 0 ? L"Edit post" : L"New post", L"blog_heading");

	auto form = addContainer(this, L"blog_form");
	addStatic(form, L"Title", L"blog_label");
	auto title = form->addChild(new wbr::Edit());
	addStatic(form, L"Text (Markdown)", L"blog_label");
	auto text = form->addChild(new wbr::TextArea());
	auto upload = form->addChild(new wbr::FileUpload(L"Insert image", L"image/png,image/jpeg,image/gif,image/webp"));
	auto error = addStatic(form, L"", L"blog_error");

	// Uploaded image is referenced at the end of the text.
	upload->uploaded() += [this, upload, text, error]() {
		const std::wstring uri = m_storeImage(upload->getData());
		if (uri.empty())
		{
			error->setText(L"Only PNG, JPEG, GIF and WebP images can be inserted.");
			return;
		}
		const std::wstring& fileName = upload->getFileName();
		const std::wstring before = rtrim(text->getText());
		error->setText(L"");
		text->setText(before + (before.empty() ? L"" : L"\n\n") + L"![" + fileName.substr(0, fileName.find_last_of(L'.')) + L"](" + uri + L")\n");
	};

	if (p)
	{
		title->setText(p->title);
		text->setText(p->text);
	}

	auto buttons = addContainer(form, L"blog_buttons");
	buttons->addChild(new wbr::Button(L"Save"))->clicked() += [this, post, title, text, error]() {
		if (trim(title->getText()).empty() || trim(text->getText()).empty())
		{
			error->setText(L"Both title and text are required.");
			return;
		}
		if (post >= 0)
		{
			// Post might have been deleted by someone else meanwhile.
			Post* edited = findPost(m_blog, post);
			if (edited)
			{
				edited->title = trim(title->getText());
				edited->text = text->getText();
				m_changed();
			}
			showPost(post);
		}
		else
		{
			const int32_t id = m_blog.nextId++;
			m_blog.posts.push_back({ trim(title->getText()), m_user, DateTime::now().getSecondsSinceEpoch(), text->getText(), id });
			m_changed();
			showPost(id);
		}
	};
	buttons->addChild(new wbr::Button(L"Cancel"))->clicked() += [this, post]() {
		if (post >= 0)
			showPost(post);
		else
			showPosts();
	};
}

}

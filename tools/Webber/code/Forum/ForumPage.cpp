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
#include "Forum/ForumData.h"
#include "Forum/ForumPage.h"
#include "Webber/Button.h"
#include "Webber/Container.h"
#include "Webber/Edit.h"
#include "Webber/Link.h"
#include "Webber/Static.h"
#include "Webber/Table.h"
#include "Webber/TextArea.h"

namespace traktor::forum
{
	namespace
	{

const wchar_t* c_separator = L"›";

std::wstring formatTime(uint64_t time)
{
	return DateTime(time).format(L"%Y-%m-%d %H:%M");
}

std::wstring formatLastPost(const Post& post)
{
	return formatTime(post.time) + L" by " + post.author;
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.forum.ForumPage", ForumPage, wbr::Container)

ForumPage::ForumPage(Forum& forum, const std::wstring& user)
:	m_forum(forum)
,	m_user(user)
{
	setStyleClass(L"forum");
	m_navigation = addContainer(this, L"forum_navigation");
	m_content = addContainer(this, L"forum_content");
	showBoards();
}

void ForumPage::showBoards()
{
	setNavigation(-1, L"");
	m_content->removeAllChildren();

	auto table = addContainer(m_content, L"forum_list");
	table->setLayout(new wbr::Table({ L"Board", L"Threads", L"Posts", L"Last post" }));

	for (int32_t i = 0; i < (int32_t)m_forum.boards.size(); ++i)
	{
		const Board& board = m_forum.boards[i];

		int32_t posts = 0;
		const Post* last = nullptr;
		for (const auto& thread : board.threads)
		{
			posts += (int32_t)thread.posts.size();
			if (!last || thread.posts.back().time > last->time)
				last = &thread.posts.back();
		}

		auto cell = table->addChild(new wbr::Container());
		cell->addChild(new wbr::Link(board.name))->clicked() += [this, i]() { showBoard(i); };
		addStatic(cell, board.description, L"forum_description");

		addStatic(table, toString((int32_t)board.threads.size()), L"forum_count");
		addStatic(table, toString(posts), L"forum_count");
		addStatic(table, last ? formatLastPost(*last) : std::wstring(L"-"), L"forum_last");
	}
}

void ForumPage::showBoard(int32_t board)
{
	const Board& b = m_forum.boards[board];

	setNavigation(board, L"");
	m_content->removeAllChildren();

	addStatic(m_content, b.name, L"forum_heading");
	m_content->addChild(new wbr::Button(L"New thread"))->clicked() += [this, board]() { showNewThread(board); };

	// Most recently active threads first.
	std::vector< int32_t > order(b.threads.size());
	for (int32_t i = 0; i < (int32_t)order.size(); ++i)
		order[i] = i;
	std::stable_sort(order.begin(), order.end(), [&](int32_t lh, int32_t rh) {
		return b.threads[lh].posts.back().time > b.threads[rh].posts.back().time;
	});

	auto table = addContainer(m_content, L"forum_list");
	table->setLayout(new wbr::Table({ L"Thread", L"Author", L"Replies", L"Last post" }));

	for (auto i : order)
	{
		const Thread& thread = b.threads[i];
		table->addChild(new wbr::Link(thread.title))->clicked() += [this, board, i]() { showThread(board, i); };
		addStatic(table, thread.posts.front().author, L"forum_author");
		addStatic(table, toString((int32_t)thread.posts.size() - 1), L"forum_count");
		addStatic(table, formatLastPost(thread.posts.back()), L"forum_last");
	}
}

void ForumPage::showThread(int32_t board, int32_t thread)
{
	const Thread& t = m_forum.boards[board].threads[thread];

	setNavigation(board, t.title);
	m_content->removeAllChildren();

	addStatic(m_content, t.title, L"forum_heading");

	for (const auto& post : t.posts)
	{
		auto epost = addContainer(m_content, L"forum_post");
		auto header = addContainer(epost, L"forum_post_header");
		addStatic(header, post.author, L"forum_author");
		addStatic(header, formatTime(post.time), L"forum_time");
		addStatic(epost, post.text, L"forum_post_text");
	}

	auto form = addContainer(m_content, L"forum_form");
	addStatic(form, L"Reply", L"forum_label");
	auto text = form->addChild(new wbr::TextArea());
	auto error = addStatic(form, L"", L"forum_error");
	form->addChild(new wbr::Button(L"Post reply"))->clicked() += [this, board, thread, text, error]() {
		if (trim(text->getText()).empty())
		{
			error->setText(L"Reply can't be empty.");
			return;
		}
		m_forum.boards[board].threads[thread].posts.push_back({ m_user, text->getText(), DateTime::now().getSecondsSinceEpoch() });
		showThread(board, thread);
	};
}

void ForumPage::showNewThread(int32_t board)
{
	setNavigation(board, L"New thread");
	m_content->removeAllChildren();

	addStatic(m_content, L"New thread", L"forum_heading");

	auto form = addContainer(m_content, L"forum_form");
	addStatic(form, L"Title", L"forum_label");
	auto title = form->addChild(new wbr::Edit());
	addStatic(form, L"Text", L"forum_label");
	auto text = form->addChild(new wbr::TextArea());
	auto error = addStatic(form, L"", L"forum_error");

	auto buttons = addContainer(form, L"forum_buttons");
	buttons->addChild(new wbr::Button(L"Post"))->clicked() += [this, board, title, text, error]() {
		if (trim(title->getText()).empty() || trim(text->getText()).empty())
		{
			error->setText(L"Both title and text are required.");
			return;
		}
		auto& threads = m_forum.boards[board].threads;
		threads.push_back({ trim(title->getText()), { { m_user, text->getText(), DateTime::now().getSecondsSinceEpoch() } } });
		showThread(board, (int32_t)threads.size() - 1);
	};
	buttons->addChild(new wbr::Button(L"Cancel"))->clicked() += [this, board]() { showBoard(board); };
}

void ForumPage::setNavigation(int32_t board, const std::wstring& title)
{
	m_navigation->removeAllChildren();
	m_navigation->addChild(new wbr::Link(L"Boards"))->clicked() += [this]() { showBoards(); };

	if (board >= 0)
	{
		addStatic(m_navigation, c_separator, L"forum_separator");
		m_navigation->addChild(new wbr::Link(m_forum.boards[board].name))->clicked() += [this, board]() { showBoard(board); };
	}

	if (!title.empty())
	{
		addStatic(m_navigation, c_separator, L"forum_separator");
		addStatic(m_navigation, title, L"forum_current");
	}
}

}

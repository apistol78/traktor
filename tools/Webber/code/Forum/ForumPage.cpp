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

/*! Index from location segment, -1 unless only digits. */
int32_t parseIndex(const std::wstring& text)
{
	if (text.empty() || text.size() > 9 || text.find_first_not_of(L"0123456789") != text.npos)
		return -1;
	return parseString< int32_t >(text);
}

std::wstring formatTime(uint64_t time)
{
	return DateTime(time).format(L"%Y-%m-%d %H:%M");
}

std::wstring formatLastPost(const Post& post)
{
	return formatTime(post.time) + L" by " + post.author;
}

AlignedVector< const Post* > visiblePosts(const Thread& thread)
{
	AlignedVector< const Post* > posts;
	for (const auto& post : thread.posts)
	{
		if (!post.deleted)
			posts.push_back(&post);
	}
	return posts;
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

ForumPage::ForumPage(Forum& forum)
:	m_forum(forum)
{
	setStyleClass(L"forum");
	m_navigation = addContainer(this, L"forum_navigation");
	m_content = addContainer(this, L"forum_content");
	showBoards();
}

void ForumPage::setUser(const std::wstring& user, bool admin)
{
	m_user = user;
	m_admin = admin;
	if (m_thread >= 0)
		showThread(m_board, m_thread);
	else if (m_board >= 0)
		showBoard(m_board);
	else
		showBoards();
}

std::wstring ForumPage::getLocation() const
{
	if (m_board < 0)
		return L"";
	else if (m_creating)
		return toString(m_board) + L"/new";
	else if (m_thread >= 0)
		return toString(m_board) + L"/" + toString(m_thread);
	else
		return toString(m_board);
}

void ForumPage::navigate(const std::wstring& location)
{
	const size_t p = location.find(L'/');
	const int32_t board = parseIndex(location.substr(0, p));
	const std::wstring rest = (p != location.npos) ? location.substr(p + 1) : L"";
	const int32_t thread = parseIndex(rest);

	if (board < 0 || board >= (int32_t)m_forum.boards.size())
		showBoards();
	else if (rest == L"new" && !m_user.empty())
		showNewThread(board);
	else if (thread < 0 || thread >= (int32_t)m_forum.boards[board].threads.size())
		showBoard(board);
	else
		showThread(board, thread);
}

void ForumPage::showBoards()
{
	m_board = m_thread = -1;
	m_creating = false;
	setNavigation(-1, L"");
	m_content->removeAllChildren();

	auto table = addContainer(m_content, L"forum_list");
	table->setLayout(new wbr::Table({ L"Board", L"Threads", L"Posts", L"Last post" }));

	for (int32_t i = 0; i < (int32_t)m_forum.boards.size(); ++i)
	{
		const Board& board = m_forum.boards[i];

		int32_t threads = 0;
		int32_t posts = 0;
		const Post* last = nullptr;
		for (const auto& thread : board.threads)
		{
			const auto visible = visiblePosts(thread);
			if (visible.empty())
				continue;
			threads++;
			posts += (int32_t)visible.size();
			if (!last || visible.back()->time > last->time)
				last = visible.back();
		}

		auto cell = table->addChild(new wbr::Container());
		cell->addChild(new wbr::Link(board.name))->clicked() += [this, i]() { showBoard(i); };
		addStatic(cell, board.description, L"forum_description");

		addStatic(table, toString(threads), L"forum_count");
		addStatic(table, toString(posts), L"forum_count");
		addStatic(table, last ? formatLastPost(*last) : std::wstring(L"-"), L"forum_last");
	}
}

void ForumPage::showBoard(int32_t board)
{
	const Board& b = m_forum.boards[board];

	m_board = board;
	m_thread = -1;
	m_creating = false;
	setNavigation(board, L"");
	m_content->removeAllChildren();

	addStatic(m_content, b.name, L"forum_heading");
	if (!m_user.empty())
		m_content->addChild(new wbr::Button(L"New thread"))->clicked() += [this, board]() { showNewThread(board); };

	// Most recently active threads first.
	std::vector< std::pair< int32_t, AlignedVector< const Post* > > > threads;
	for (int32_t i = 0; i < (int32_t)b.threads.size(); ++i)
	{
		auto visible = visiblePosts(b.threads[i]);
		if (!visible.empty())
			threads.push_back({ i, visible });
	}
	std::stable_sort(threads.begin(), threads.end(), [](const auto& lh, const auto& rh) {
		return lh.second.back()->time > rh.second.back()->time;
	});

	auto table = addContainer(m_content, L"forum_list");
	table->setLayout(new wbr::Table({ L"Thread", L"Author", L"Replies", L"Last post" }));

	for (const auto& thread : threads)
	{
		const int32_t i = thread.first;
		const auto& posts = thread.second;
		table->addChild(new wbr::Link(b.threads[i].title))->clicked() += [this, board, i]() { showThread(board, i); };
		addStatic(table, posts.front()->author, L"forum_author");
		addStatic(table, toString((int32_t)posts.size() - 1), L"forum_count");
		addStatic(table, formatLastPost(*posts.back()), L"forum_last");
	}
}

void ForumPage::showThread(int32_t board, int32_t thread)
{
	const Thread& t = m_forum.boards[board].threads[thread];
	if (visiblePosts(t).empty())
	{
		showBoard(board);
		return;
	}

	m_board = board;
	m_thread = thread;
	m_creating = false;
	setNavigation(board, t.title);
	m_content->removeAllChildren();

	addStatic(m_content, t.title, L"forum_heading");

	for (int32_t i = 0; i < (int32_t)t.posts.size(); ++i)
	{
		const Post& post = t.posts[i];
		if (post.deleted)
			continue;

		auto epost = addContainer(m_content, L"forum_post");
		auto header = addContainer(epost, L"forum_post_header");
		addStatic(header, post.author, L"forum_author");
		auto meta = addContainer(header, L"forum_post_meta");
		addStatic(meta, formatTime(post.time), L"forum_time");
		if (m_admin || (!m_user.empty() && post.author == m_user))
		{
			meta->addChild(new wbr::Link(L"Delete"))->clicked() += [this, board, thread, i]() {
				m_forum.boards[board].threads[thread].posts[i].deleted = true;
				showThread(board, thread);
			};
		}
		addStatic(epost, post.text, L"forum_post_text");
	}

	if (m_user.empty())
		return;

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
	m_board = board;
	m_thread = -1;
	m_creating = true;
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

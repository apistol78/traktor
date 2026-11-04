/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
#include <string>
#include "Webber/Container.h"

namespace traktor::forum
{

class Forum;

/*! Forum page; navigates between boards, threads and the new thread form.
 * \ingroup Webber
 */
class ForumPage : public wbr::Container
{
	T_RTTI_CLASS;

public:
	/*! \param changed Called after the forum has been modified. */
	explicit ForumPage(Forum& forum, const std::function< void() >& changed);

	/*! Set user and redraw current view.
	 * \param user Author of posts made through this page, empty for guest.
	 * \param admin Admins can delete any post.
	 */
	void setUser(const std::wstring& user, bool admin);

	/*! Location of current view; "", "<board>", "<board>/<thread>" or "<board>/new". */
	std::wstring getLocation() const;

	/*! Show view at location; invalid locations show the closest valid view. */
	void navigate(const std::wstring& location);

	void showBoards();

	void showBoard(int32_t board);

	/*! \param thread Thread id. */
	void showThread(int32_t board, int32_t thread);

private:
	Forum& m_forum;
	std::function< void() > m_changed;
	std::wstring m_user;
	bool m_admin = false;
	int32_t m_board = -1;	//!< Board being shown, -1 when listing boards.
	int32_t m_thread = -1;	//!< Id of thread being shown, -1 when not showing a thread.
	bool m_creating = false;	//!< New thread form is shown for m_board.
	Ref< wbr::Container > m_navigation;
	Ref< wbr::Container > m_content;

	void showNewThread(int32_t board);

	void setNavigation(int32_t board, const std::wstring& title);
};

}

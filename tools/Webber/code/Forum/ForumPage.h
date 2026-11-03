/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Webber/Container.h"

namespace traktor::forum
{

struct Forum;

/*! Forum page; navigates between boards, threads and the new thread form.
 * \ingroup Webber
 */
class ForumPage : public wbr::Container
{
	T_RTTI_CLASS;

public:
	/*! \param user Author of posts made through this page. */
	explicit ForumPage(Forum& forum, const std::wstring& user);

	void showBoards();

private:
	Forum& m_forum;
	std::wstring m_user;
	Ref< wbr::Container > m_navigation;
	Ref< wbr::Container > m_content;

	void showBoard(int32_t board);

	void showThread(int32_t board, int32_t thread);

	void showNewThread(int32_t board);

	void setNavigation(int32_t board, const std::wstring& title);
};

}

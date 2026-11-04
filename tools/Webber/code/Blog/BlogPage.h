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

namespace traktor::blog
{

struct Blog;

/*! Blog page; lists posts, shows a single post and edits posts.
 * \ingroup Webber
 */
class BlogPage : public wbr::Container
{
	T_RTTI_CLASS;

public:
	explicit BlogPage(Blog& blog);

	/*! Set user and redraw current view.
	 * \param user Author of posts made through this page, empty for guest.
	 * \param admin Admins can add, edit and delete posts.
	 */
	void setUser(const std::wstring& user, bool admin);

	/*! Location of current view; "", "<post>", "<post>/edit" or "new". */
	std::wstring getLocation() const;

	/*! Show view at location; invalid locations show the closest valid view. */
	void navigate(const std::wstring& location);

	void showPosts();

	void showPost(int32_t post);

private:
	Blog& m_blog;
	std::wstring m_user;
	bool m_admin = false;
	int32_t m_post = -1;	//!< Post being shown, -1 when listing posts.
	bool m_editing = false;	//!< Editor is shown for m_post.

	/*! \param post Post to edit, -1 for a new post. */
	void showEditor(int32_t post);
};

}

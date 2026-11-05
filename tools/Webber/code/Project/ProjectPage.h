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
#include "Core/Containers/AlignedVector.h"
#include "Webber/Container.h"

namespace traktor::project
{

class Projects;
struct Project;

/*! Project page; lists project summaries, shows a project's description and edits projects.
 * \ingroup Webber
 */
class ProjectPage : public wbr::Container
{
	T_RTTI_CLASS;

public:
	/*! \param changed Called after projects have been modified.
	 * \param storeImage Stores an uploaded image and returns its URI, empty if it isn't a supported image.
	 */
	explicit ProjectPage(Projects& projects, const std::function< void() >& changed, const std::function< std::wstring(const AlignedVector< uint8_t >&) >& storeImage);

	/*! Set user and redraw current view.
	 * \param user Author of projects created through this page, empty for guest.
	 * \param admin Admins can edit and delete all projects.
	 */
	void setUser(const std::wstring& user, bool admin);

	/*! Location of current view; "", "<project>", "<project>/edit" or "new". */
	std::wstring getLocation() const;

	/*! Show view at location; invalid locations show the closest valid view. */
	void navigate(const std::wstring& location);

	void showProjects();

	/*! \param project Project id. */
	void showProject(int32_t project);

private:
	Projects& m_projects;
	std::function< void() > m_changed;
	std::function< std::wstring(const AlignedVector< uint8_t >&) > m_storeImage;
	std::wstring m_user;
	bool m_admin = false;
	int32_t m_project = -1;	//!< Id of project being shown, -1 when listing projects.
	bool m_editing = false;	//!< Editor is shown for m_project.

	/*! \param project Id of project to edit, -1 for a new project. */
	void showEditor(int32_t project);

	/*! Author and admins can edit and delete a project. */
	bool canEdit(const Project& project) const;
};

}

/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Guid.h"
#include "Core/Ref.h"
#include "Ui/ConfigDialog.h"

#include <string>
#include <utility>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class ContentChangeEvent;
class Edit;
class ListBox;
class MouseDoubleClickEvent;

}

namespace traktor::scene
{

/*! Pick one of a scene's entities.
 * \ingroup Scene
 */
class T_DLLCLASS BrowseEntityDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent, const AlignedVector< std::pair< std::wstring, Guid > >& entities, const Guid& current);

	Guid getSelected() const;

private:
	AlignedVector< std::pair< std::wstring, Guid > > m_entities;
	AlignedVector< Guid > m_shown;
	Guid m_current;
	Ref< ui::Edit > m_filter;
	Ref< ui::ListBox > m_list;

	void fill();

	void eventFilterChange(ui::ContentChangeEvent* event);

	void eventDoubleClick(ui::MouseDoubleClickEvent* event);
};

}

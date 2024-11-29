/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Io/Path.h"
#include "Ui/PropertyList/PropertyItem.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class Edit;
class MiniButton;

/*! File property item.
 * \ingroup UI
 */
class T_DLLCLASS FilePropertyItem : public PropertyItem
{
	T_RTTI_CLASS;

public:
	explicit FilePropertyItem(const std::wstring& text, const Path& path, bool directory);

	void setPath(const Path& path);

	const Path& getPath() const;

	bool isDirectory() const;

protected:
	virtual void createInPlaceControls(PropertyList* parent) override;

	virtual void destroyInPlaceControls() override;

	virtual void resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects) override;

	virtual void mouseButtonDown(MouseButtonDownEvent* event) override;

	virtual void paintValue(PropertyList* parent, Canvas& canvas, const Rect& rc) override;

	virtual bool copy() override;

	virtual bool paste() override;

private:
	Ref< Edit > m_editor;
	Ref< MiniButton > m_buttonEdit;
	Path m_path;
	bool m_directory;

	void eventEditFocus(FocusEvent* event);

	void eventClick(ButtonClickEvent* event);
};

}

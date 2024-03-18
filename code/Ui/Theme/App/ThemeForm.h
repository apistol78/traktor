/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Io/Path.h"
#include "Ui/Form.h"

namespace traktor
{

class CommandLine;

}

namespace traktor::ui
{

class GridView;
class ShortcutEvent;
class ShortcutTable;
class StyleSheet;
class ToolBar;
class ToolBarButtonClickEvent;
class TreeView;
class TreeViewContentChangeEvent;
class TreeViewItem;
class TreeViewItemActivateEvent;

class ThemeForm : public Form
{
	T_RTTI_CLASS;

public:
	bool create(const CommandLine& cmdLine);

private:
	Ref< ShortcutTable > m_shortcutTable;
	Ref< ToolBar > m_menuBar;
	Ref< TreeView > m_treeTheme;
	Ref< TreeViewItem > m_pinnedElementItem;
	Ref< GridView > m_gridPalette;
	Ref< GridView > m_gridValues;
	Ref< Container > m_containerPreview;
	Ref< StyleSheet > m_styleSheet;
	Path m_styleSheetPath;
	uint32_t m_styleSheetHash = 0;

	void updateTree();

	void updatePalette();

	void updateValues();

	void updatePreview();

	void updateTitle();

	bool checkModified() const;

	void handleCommand(const Command& command);

	void eventClose(CloseEvent*);

	void eventShortcut(ShortcutEvent* event);

	void eventMenuClick(ToolBarButtonClickEvent* event);

	void eventTreeSelectionChange(SelectionChangeEvent* event);

	void eventTreeActivateItem(TreeViewItemActivateEvent* event);

	void eventTreeButtonDown(MouseButtonDownEvent* event);

	void eventTreeChange(TreeViewContentChangeEvent* event);

	void eventPaletteDoubleClick(MouseDoubleClickEvent* event);
};

}

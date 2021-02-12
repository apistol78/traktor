#pragma once

#include "Core/Io/Path.h"
#include "Ui/Form.h"

namespace traktor
{
	namespace ui
	{

class GridView;
class StyleSheet;
class ToolBar;
class ToolBarButtonClickEvent;
class TreeView;
class TreeViewContentChangeEvent;
class TreeViewItemActivateEvent;

class ThemeForm : public Form
{
	T_RTTI_CLASS;

public:
	bool create();

private:
	Ref< ToolBar > m_menuBar;
	Ref< TreeView > m_treeTheme;
	Ref< GridView > m_gridPalette;
	Ref< Container > m_containerPreview;
	Ref< StyleSheet > m_styleSheet;
	Path m_styleSheetPath;
	uint32_t m_styleSheetHash = 0;

	void updateTree();

	void updatePalette();

	void updatePreview();

	void updateTitle();

	bool checkModified() const;

	void handleCommand(const Command& command);

	void eventTimer(TimerEvent*);

	void eventClose(CloseEvent*);

	void eventMenuClick(ToolBarButtonClickEvent* event);

	void eventTreeSelectionChange(SelectionChangeEvent* event);

	void eventTreeActivateItem(TreeViewItemActivateEvent* event);

	void eventTreeButtonDown(MouseButtonDownEvent* event);

	void eventTreeChange(TreeViewContentChangeEvent* event);

	void eventPaletteDoubleClick(MouseDoubleClickEvent* event);
};

	}
}

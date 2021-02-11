#pragma once

#include "Core/Io/Path.h"
#include "Ui/Form.h"

namespace traktor
{
	namespace ui
	{

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
	Ref< Container > m_containerPreview;
	Ref< StyleSheet > m_styleSheet;
	Path m_styleSheetPath;

	void updateTree();

	void updatePreview();

	void handleCommand(const Command& command);

	void eventMenuClick(ToolBarButtonClickEvent* event);

	void eventTreeSelectionChange(SelectionChangeEvent* event);

	void eventTreeActivateItem(TreeViewItemActivateEvent* event);

	void eventTreeButtonDown(MouseButtonDownEvent* event);

	void eventTreeChange(TreeViewContentChangeEvent* event);
};

	}
}

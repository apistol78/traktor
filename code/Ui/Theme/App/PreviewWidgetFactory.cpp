#include "Ui/TableLayout.h"
#include "Ui/Theme/App/PreviewWidgetFactory.h"

// Preview widgets.
#include "Ui/Button.h"
#include "Ui/Dock.h"
#include "Ui/DropDown.h"
#include "Ui/Edit.h"
#include "Ui/MenuItem.h"
#include "Ui/MenuShell.h"
#include "Ui/MiniButton.h"
#include "Ui/Panel.h"
#include "Ui/ProgressBar.h"
#include "Ui/ScrollBar.h"
#include "Ui/ShortcutEdit.h"
#include "Ui/Tab.h"
#include "Ui/TabPage.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridView.h"
#include "Ui/ListBox/ListBox.h"
#include "Ui/LogList/LogList.h"
#include "Ui/PreviewList/PreviewList.h"
#include "Ui/PreviewList/PreviewItem.h"
#include "Ui/PreviewList/PreviewItems.h"
#include "Ui/PropertyList/NumericPropertyItem.h"
#include "Ui/PropertyList/PropertyList.h"
#include "Ui/RichEdit/RichEdit.h"
#include "Ui/StatusBar/StatusBar.h"
#include "Ui/SyntaxRichEdit/SyntaxRichEdit.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarDropDown.h"
#include "Ui/ToolBar/ToolBarDropMenu.h"
#include "Ui/ToolBar/ToolBarSeparator.h"
#include "Ui/TreeView/TreeView.h"
#include "Ui/TreeView/TreeViewItem.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

const wchar_t* lorem =
{
	L"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec fermentum dui pellentesque, rutrum sem nec,\n"
	L"vehicula nisi. Nulla facilisi. Quisque non viverra neque. Mauris molestie tellus sit amet justo feugiat, eu\n"
	L"tincidunt nibh lacinia. Nullam gravida pellentesque mi vel lacinia. Pellentesque eleifend feugiat mi, sit\n"
	L"amet venenatis tortor sagittis et. Suspendisse posuere tellus non felis posuere, sed porta justo dapibus.\n"
	L"Curabitur et ultrices justo. Donec feugiat ultrices ligula et bibendum. Aliquam porta mollis lacus, ut hendrerit\n"
	L"massa tempus non. Aenean quis tortor ac elit fringilla auctor sed vitae arcu. Proin quis elementum elit.\n"
	L"Vestibulum porttitor porttitor accumsan. Praesent nulla risus, tempor eget dui a, vestibulum volutpat felis.\n"
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.PreviewWidgetFactory", PreviewWidgetFactory, Object)

Ref< Widget > PreviewWidgetFactory::create(Widget* parent, const StyleSheet* styleSheet, const std::wstring& typeName) const
{
	if (typeName == L"traktor.ui.Button")
	{
		Ref< Button > button = new Button();
		button->setStyleSheet(styleSheet);
		button->create(parent, L"Preview");
		return button;
	}
	else if (typeName == L"traktor.ui.Dock")
	{
		Ref< Dock > dock = new Dock();
		dock->setStyleSheet(styleSheet);
		dock->create(parent);
		return dock;
	}
	else if (typeName == L"traktor.ui.DropDown")
	{
		Ref< DropDown > dropDown = new DropDown();
		dropDown->setStyleSheet(styleSheet);
		dropDown->create(parent);
		dropDown->add(L"First");
		dropDown->add(L"Second");
		dropDown->add(L"Third");
		return dropDown;
	}
	else if (typeName == L"traktor.ui.Edit")
	{
		Ref< Edit > edit = new Edit();
		edit->setStyleSheet(styleSheet);
		edit->create(parent, L"Preview");
		return edit;
	}
	else if (typeName == L"traktor.ui.GridView")
	{
		Ref< GridView > gridView = new GridView();
		gridView->setStyleSheet(styleSheet);
		gridView->create(parent, ui::WsAccelerated | ui::GridView::WsColumnHeader);
		gridView->addColumn(new GridColumn(L"First", 100));
		gridView->addColumn(new GridColumn(L"Second", 100));
		gridView->addColumn(new GridColumn(L"Third", 100));

		Ref< GridRow > row1 = new GridRow();
		row1->add(new GridItem(L"A1"));
		row1->add(new GridItem(L"B1"));
		row1->add(new GridItem(L"C1"));
		gridView->addRow(row1);

		Ref< GridRow > row2 = new GridRow();
		row2->add(new GridItem(L"A2"));
		row2->add(new GridItem(L"B2"));
		row2->add(new GridItem(L"C2"));
		gridView->addRow(row2);

		return gridView;
	}
	else if (typeName == L"traktor.ui.ListBox")
	{
		Ref< ListBox > listBox = new ListBox();
		listBox->setStyleSheet(styleSheet);
		listBox->create(parent);
		listBox->add(L"First");
		listBox->add(L"Second");
		listBox->add(L"Third");
		return listBox;
	}
	else if (typeName == L"traktor.ui.LogList")
	{
		Ref< LogList > logList = new LogList();
		logList->setStyleSheet(styleSheet);
		logList->create(parent, ui::WsAccelerated, nullptr);
		logList->add(0, LogList::LvInfo, L"First");
		logList->add(0, LogList::LvWarning, L"Second");
		logList->add(0, LogList::LvError, L"Third");
		logList->add(0, LogList::LvDebug, L"Fourth");
		return logList;
	}
	else if (typeName == L"traktor.ui.MenuItem" || typeName == L"traktor.ui.MenuShell")
	{
		Ref< MenuShell > menuShell = new MenuShell();
		menuShell->setStyleSheet(styleSheet);
		menuShell->create(parent, 4);
		menuShell->add(new MenuItem(L"First"));
		menuShell->add(new MenuItem(L"Second"));
		menuShell->add(new MenuItem(L"Third"));
		menuShell->add(new MenuItem(L"Fourth"));
		menuShell->add(new MenuItem(L"Fifth"));
		return menuShell;
	}
	else if (typeName == L"traktor.ui.MiniButton")
	{
		Ref< MiniButton > miniButton = new MiniButton();
		miniButton->setStyleSheet(styleSheet);
		miniButton->create(parent, L"Preview");
		return miniButton;
	}
	else if (typeName == L"traktor.ui.Panel")
	{
		Ref< Panel > panel = new Panel();
		panel->setStyleSheet(styleSheet);
		panel->create(parent, L"Preview", nullptr);
		return panel;
	}
	else if (typeName == L"traktor.ui.PreviewList")
	{
		Ref< PreviewList > previewList = new PreviewList();
		previewList->setStyleSheet(styleSheet);
		previewList->create(parent, WsAccelerated);

		Ref< PreviewItems > items = new PreviewItems();
		items->add(new PreviewItem(L"First"));
		items->add(new PreviewItem(L"Second"));
		items->add(new PreviewItem(L"Third"));
		previewList->setItems(items);

		return previewList;
	}
	else if (typeName == L"traktor.ui.ProgressBar")
	{
		Ref< ProgressBar > progressBar = new ProgressBar();
		progressBar->setStyleSheet(styleSheet);
		progressBar->create(parent, WsAccelerated, 0, 100);
		progressBar->setProgress(50);
		progressBar->setText(L"Preview");
		return progressBar;
	}
	else if (typeName == L"traktor.ui.PropertyList")
	{
		Ref< PropertyList > propertyList = new PropertyList();
		propertyList->setStyleSheet(styleSheet);
		propertyList->create(parent);
		propertyList->addPropertyItem(new NumericPropertyItem(L"First", 100, 0, 200, true, false, NumericPropertyItem::RpNormal));
		return propertyList;
	}
	else if (typeName == L"traktor.ui.RichEdit")
	{
		Ref< RichEdit > richEdit = new RichEdit();
		richEdit->setStyleSheet(styleSheet);
		richEdit->create(parent, lorem);
		return richEdit;
	}
	else if (typeName == L"traktor.ui.ScrollBar")
	{
		Ref< ScrollBar > scrollBar = new ScrollBar();
		scrollBar->setStyleSheet(styleSheet);
		scrollBar->create(parent);
		return scrollBar;
	}
	else if (typeName == L"traktor.ui.ShortcutEdit")
	{
		Ref< ShortcutEdit > shortcutEdit = new ShortcutEdit();
		shortcutEdit->setStyleSheet(styleSheet);
		shortcutEdit->create(parent, 0, VkSpace);
		return shortcutEdit;
	}
	else if (typeName == L"traktor.ui.StatusBar")
	{
		Ref< Container > container = new Container();
		container->setStyleSheet(styleSheet);
		container->create(parent, WsNone, new TableLayout(L"100%", L"*", 0, 16));

		Ref< StatusBar > statusBar1 = new StatusBar();
		statusBar1->create(container);
		statusBar1->setText(L"Preview");

		Ref< StatusBar > statusBar2 = new StatusBar();
		statusBar2->create(container);
		statusBar2->setText(L"Preview");
		statusBar2->setAlert(true);

		return container;
	}
	else if (typeName == L"traktor.ui.SyntaxRichEdit")
	{
		Ref< SyntaxRichEdit > richEdit = new SyntaxRichEdit();
		richEdit->setStyleSheet(styleSheet);
		richEdit->create(parent, lorem);
		return richEdit;
	}
	else if (typeName == L"traktor.ui.Tab")
	{
		Ref< Tab > tab = new Tab();
		tab->setStyleSheet(styleSheet);
		tab->create(parent);

		Ref< TabPage > page1 = new TabPage();
		page1->create(tab, L"First", nullptr);
		tab->addPage(page1);

		Ref< TabPage > page2 = new TabPage();
		page2->create(tab, L"Second", nullptr);
		tab->addPage(page2);

		Ref< TabPage > page3 = new TabPage();
		page3->create(tab, L"Third", nullptr);
		tab->addPage(page3);

		return tab;
	}
	else if (typeName == L"traktor.ui.ToolBar")
	{
		Ref< ToolBar > toolBar = new ToolBar();
		toolBar->setStyleSheet(styleSheet);
		toolBar->create(parent);

		toolBar->addItem(new ToolBarButton(L"Button", Command()));
		toolBar->addItem(new ToolBarDropDown(Command(), 100, L"DropDown"));
		toolBar->addItem(new ToolBarDropMenu(100, L"DropMenu", true, L"DropMenu"));
		toolBar->addItem(new ToolBarSeparator());

		return toolBar;
	}
	else if (typeName == L"traktor.ui.TreeView")
	{
		Ref< TreeView > treeView = new TreeView();
		treeView->setStyleSheet(styleSheet);
		treeView->create(parent, TreeView::WsDefault);

		Ref< TreeViewItem > treeItem = treeView->createItem(nullptr, L"Root", 0);
		treeView->createItem(treeItem, L"First", 0);
		treeView->createItem(treeItem, L"Second", 0);
		treeView->createItem(treeItem, L"Third", 0);

		return treeView;
	}
	else
		return nullptr;
}

	}
}

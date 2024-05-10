/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/TableLayout.h"
#include "Ui/Theme/App/PreviewWidgetFactory.h"

// Preview widgets.
#include "Ui/Button.h"
#include "Ui/Dock.h"
#include "Ui/DockPane.h"
#include "Ui/DropDown.h"
#include "Ui/Edit.h"
#include "Ui/MenuItem.h"
#include "Ui/MenuShell.h"
#include "Ui/MiniButton.h"
#include "Ui/Panel.h"
#include "Ui/ProgressBar.h"
#include "Ui/ScrollBar.h"
#include "Ui/ShortcutEdit.h"
#include "Ui/Slider.h"
#include "Ui/Static.h"
#include "Ui/StyleBitmap.h"
#include "Ui/Tab.h"
#include "Ui/TabPage.h"
#include "Ui/BuildChart/BuildChartControl.h"
#include "Ui/Envelope/DefaultEnvelopeEvaluator.h"
#include "Ui/Envelope/EnvelopeControl.h"
#include "Ui/Envelope/EnvelopeKey.h"
#include "Ui/Graph/DefaultNodeShape.h"
#include "Ui/Graph/Edge.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/Node.h"
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
#include "Ui/Sequencer/Sequence.h"
#include "Ui/Sequencer/SequenceGroup.h"
#include "Ui/Sequencer/SequencerControl.h"
#include "Ui/StatusBar/StatusBar.h"
#include "Ui/SyntaxRichEdit/SyntaxRichEdit.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarDropDown.h"
#include "Ui/ToolBar/ToolBarDropMenu.h"
#include "Ui/ToolBar/ToolBarSeparator.h"
#include "Ui/TreeView/TreeView.h"
#include "Ui/TreeView/TreeViewItem.h"
#include "Ui/Sequencer/Range.h"
#include "Ui/Sequencer/Sequence.h"
#include "Ui/Sequencer/SequenceGroup.h"
#include "Ui/Sequencer/SequencerControl.h"
#include "Ui/Sequencer/Tick.h"
#include "Ui/Sequencer/Track.h"

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
	if (typeName == L"traktor.ui.BuildChartControl")
	{
		Ref< BuildChartControl > buildChart = new BuildChartControl();
		buildChart->setStyleSheet(styleSheet);
		buildChart->create(parent, 4);
		buildChart->addTask(0, L"Task", Color4ub(255, 0, 0, 255), 0.0, 2.0);
		return buildChart;
	}
	else if (typeName == L"traktor.ui.Button")
	{
		Ref< Button > button = new Button();
		button->setStyleSheet(styleSheet);
		button->create(parent, L"Preview");
		return button;
	}
	else if (typeName == L"traktor.ui.Dock" || typeName == L"traktor.ui.DockPane")
	{
		Ref< Dock > dock = new Dock();
		dock->setStyleSheet(styleSheet);
		dock->create(parent);

		Ref< Widget > widget = new Widget();
		widget->create(dock);
		widget->setText(L"Dock");
		dock->getPane()->dock(widget);

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
	else if (typeName == L"traktor.ui.GraphControl" || typeName == L"traktor.ui.Edge" || typeName == L"traktor.ui.Node" || typeName == L"traktor.ui.INodeShape")
	{
		Ref< GraphControl > graphControl = new GraphControl();
		graphControl->setStyleSheet(styleSheet);
		graphControl->create(parent);
		graphControl->setText(L"GRAPH");

		Ref< Edge > edge = new Edge();
		edge->setText(L"EDGE");

		{
			Node* node = graphControl->createNode(L"First", L"Info", UnitPoint(0_ut, 0_ut), new DefaultNodeShape(DefaultNodeShape::StDefault));
			node->createInputPin(L"Input", Guid(), true, false);
			edge->setSourcePin(node->createOutputPin(L"Output", Guid()));
		}

		{
			Node* node = graphControl->createNode(L"Second", L"Info", UnitPoint(100_ut, 0_ut), new DefaultNodeShape(DefaultNodeShape::StExternal));
			edge->setDestinationPin(node->createInputPin(L"Input", Guid(), true, false));
			node->createOutputPin(L"Output", Guid());
		}

		graphControl->addEdge(edge);

		return graphControl;
	}
	else if (typeName == L"traktor.ui.GridView" || typeName == L"traktor.ui.GridHeader" || typeName == L"traktor.ui.GridRow" || typeName == L"traktor.ui.GridItem")
	{
		Ref< GridView > gridView = new GridView();
		gridView->setStyleSheet(styleSheet);
		gridView->create(parent, ui::WsAccelerated | ui::GridView::WsColumnHeader);
		gridView->addColumn(new GridColumn(L"First", 100_ut));
		gridView->addColumn(new GridColumn(L"Second", 100_ut));
		gridView->addColumn(new GridColumn(L"Third", 100_ut));

		Ref< GridRow > row1 = new GridRow();
		row1->add(new GridItem(L"A1"));
		row1->add(new GridItem(L"B1"));
		row1->add(new GridItem(L"C1"));
		gridView->addRow(row1);

		Ref< GridRow > row2 = new GridRow();
		row2->add(new GridItem(L"A2"));
		row2->add(new GridItem(L"B2"));
		row2->add(new GridItem(L"C2"));

		Ref< GridRow > cr1 = new GridRow();
		cr1->add(new GridItem(L"Child"));
		Ref< GridRow > cr2 = new GridRow();
		cr2->add(new GridItem(L"Child"));
		cr1->addChild(cr2);
		Ref< GridRow > cr3 = new GridRow();
		cr3->add(new GridItem(L"Child"));
		cr2->addChild(cr3);
		Ref< GridRow > cr4 = new GridRow();
		cr4->add(new GridItem(L"Child"));
		cr3->addChild(cr4);
		row2->addChild(cr1);

		gridView->addRow(row2);

		return gridView;
	}
	else if (typeName == L"traktor.ui.ListBox" || typeName == L"traktor.ui.ListBoxItem")
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
	else if (typeName == L"traktor.ui.PreviewList" || typeName == L"traktor.ui.PreviewItem")
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
		Ref< Container > container = new Container();
		container->setStyleSheet(styleSheet);
		container->create(parent, WsNone, new TableLayout(L"100%", L"*", 0_ut, 16_ut));

		Ref< ProgressBar > progressBar0 = new ProgressBar();
		progressBar0->setStyleSheet(styleSheet);
		progressBar0->create(container, WsAccelerated, 0, 100);
		progressBar0->setProgress(50);
		progressBar0->setText(L"Preview");

		Ref< ProgressBar > progressBar1 = new ProgressBar();
		progressBar1->setStyleSheet(styleSheet);
		progressBar1->create(container, WsAccelerated, 0, 0);
		progressBar1->setProgress(50);
		progressBar1->setText(L"Preview");

		return container;
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
	else if (typeName == L"traktor.ui.Sequence" || typeName == L"traktor.ui.SequenceGroup" || typeName == L"traktor.ui.SequencerControl" || typeName == L"traktor.ui.Tick" || typeName == L"traktor.ui.Track")
	{
		Ref< Sequence > item = new Sequence(L"Item");
		item->addKey(new Tick(100, true));
		item->addKey(new Track(500, 2000, true));

		Ref< SequencerControl > sequencer = new SequencerControl();
		sequencer->setStyleSheet(styleSheet);
		sequencer->create(parent);
		sequencer->addSequenceItem(item);
		sequencer->addSequenceItem(new SequenceGroup(L"Group"));
		return sequencer;
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
	else if (typeName == L"traktor.ui.Slider")
	{
		Ref< Slider > slider = new Slider();
		slider->setStyleSheet(styleSheet);
		slider->create(parent);
		slider->setRange(0, 100);
		slider->setValue(50);
		return slider;
	}
	else if (typeName == L"traktor.ui.Static")
	{
		Ref< Static > statik = new Static();
		statik->setStyleSheet(styleSheet);
		statik->create(parent, L"Preview");
		return statik;
	}
	else if (typeName == L"traktor.ui.StatusBar")
	{
		Ref< Container > container = new Container();
		container->setStyleSheet(styleSheet);
		container->create(parent, WsNone, new TableLayout(L"100%", L"*", 0_ut, 16_ut));

		Ref< StatusBar > statusBar1 = new StatusBar();
		statusBar1->create(container);
		statusBar1->addColumn(-1);
		statusBar1->setText(0, L"Preview");

		Ref< StatusBar > statusBar2 = new StatusBar();
		statusBar2->create(container);
		statusBar1->addColumn(-1);
		statusBar2->setText(0, L"Preview");
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
		toolBar->addItem(new ToolBarDropDown(Command(), 100_ut, L"DropDown"));
		toolBar->addItem(new ToolBarDropMenu(100_ut, L"DropMenu", true, L"DropMenu"));
		toolBar->addItem(new ToolBarSeparator());

		return toolBar;
	}
	else if (typeName == L"traktor.ui.TreeView" || typeName == L"traktor.ui.TreeViewItem")
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
	else if (
		typeName == L"traktor.ui.SequencerControl" ||
		typeName == L"traktor.ui.Sequence" ||
		typeName == L"traktor.ui.SequenceGroup" ||
		typeName == L"traktor.ui.Tick" ||
		typeName == L"traktor.ui.Range" ||
		typeName == L"traktor.ui.Track"
	)
	{
		Ref< ui::IBitmap > layerDelete = new ui::StyleBitmap(L"Spray.LayerDelete");
		Ref< ui::IBitmap > layerVisible = new ui::StyleBitmap(L"Spray.LayerVisible");
		Ref< ui::IBitmap > layerHidden = new ui::StyleBitmap(L"Spray.LayerHidden");

		Ref< SequencerControl > sequencerControl = new SequencerControl();
		sequencerControl->setStyleSheet(styleSheet);
		sequencerControl->create(parent);
		sequencerControl->addSequenceItem(new Sequence(L"First"));

		Ref< Sequence > sequence = new Sequence(L"Second");
		sequence->addButton(layerDelete, layerDelete, ui::Command(L"Effect.Editor.DeleteLayer"));
		sequence->addButton(layerVisible, layerHidden, ui::Command(L"Effect.Editor.ToggleLayerVisible"), true);
		sequence->addKey(new Tick(100, true));
		sequence->addKey(new Range(200, 500, true));
		sequence->addKey(new Track(600, 900, true));
		sequencerControl->addSequenceItem(sequence);

		Ref< SequenceGroup > group = new SequenceGroup(L"Third");
		group->addChildItem(new Sequence(L"Fourth"));
		group->addChildItem(new Sequence(L"Fifth"));
		sequencerControl->addSequenceItem(group);

		return sequencerControl;
	}
	else if (typeName == L"traktor.ui.EnvelopeControl")
	{
		Ref< EnvelopeControl > envelopeControl = new EnvelopeControl();
		envelopeControl->setStyleSheet(styleSheet);
		envelopeControl->create(parent, new DefaultEnvelopeEvaluator< HermiteEvaluator >());
		envelopeControl->insertKey(new EnvelopeKey(0.0f, 0.0f));
		envelopeControl->insertKey(new EnvelopeKey(1.0f, 1.0f));

		return envelopeControl;
	}
	else
		return nullptr;
}

	}
}

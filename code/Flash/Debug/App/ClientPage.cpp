/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Format.h"
#include "Core/Misc/TString.h"
#include "Flash/Debug/ButtonInstanceDebugInfo.h"
#include "Flash/Debug/CaptureControl.h"
#include "Flash/Debug/EditInstanceDebugInfo.h"
#include "Flash/Debug/MorphShapeInstanceDebugInfo.h"
#include "Flash/Debug/MovieDebugInfo.h"
#include "Flash/Debug/ShapeInstanceDebugInfo.h"
#include "Flash/Debug/SpriteInstanceDebugInfo.h"
#include "Flash/Debug/TextInstanceDebugInfo.h"
#include "Flash/Debug/App/ClientPage.h"
#include "Flash/Debug/App/DebugView.h"
#include "Ui/Application.h"
#include "Ui/FileDialog.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridView.h"
#include "Ui/Custom/StatusBar/StatusBar.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"
#include "Ui/Custom/TreeView/TreeView.h"
#include "Ui/Custom/TreeView/TreeViewItem.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

namespace traktor
{
	namespace flash
	{

bool ClientPage::create(ui::Widget* parent, net::BidirectionalObjectTransport* transport)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

	m_toolBar = new ui::custom::ToolBar();
	m_toolBar->create(this);

	m_toolBar->addItem(new ui::custom::ToolBarButton(L"Load", ui::Command(L"Traktor.Flash.Load")));
	m_toolBar->addItem(new ui::custom::ToolBarButton(L"Save", ui::Command(L"Traktor.Flash.Save")));
	m_toolBar->addItem(new ui::custom::ToolBarSeparator());
	m_toolBar->addItem(new ui::custom::ToolBarButton(L"Capture", ui::Command(L"Traktor.Flash.CaptureSingle")));
	m_toolBar->addItem(new ui::custom::ToolBarSeparator());
	m_toolBar->addItem(new ui::custom::ToolBarButton(L"Selected Only", ui::Command(L"Traktor.Flash.ToggleShowSelectedOnly"), ui::custom::ToolBarButton::BsText | ui::custom::ToolBarButton::BsToggle));
	m_toolBar->addItem(new ui::custom::ToolBarButton(L"Outline", ui::Command(L"Traktor.Flash.ToggleShowOutline"), ui::custom::ToolBarButton::BsText | ui::custom::ToolBarButton::BsToggled));

	m_toolBar->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &ClientPage::eventToolBarClick);

	Ref< ui::custom::Splitter > splitter = new ui::custom::Splitter();
	splitter->create(this, true, 30, true);

	Ref< ui::custom::Splitter > splitter2 = new ui::custom::Splitter();
	splitter2->create(splitter, false, 50, true);

	m_debugTree = new ui::custom::TreeView();
	m_debugTree->create(splitter2, ui::custom::TreeView::WsTreeButtons | ui::custom::TreeView::WsTreeLines | ui::WsAccelerated);
	m_debugTree->addEventHandler< ui::SelectionChangeEvent >(this, &ClientPage::eventDebugTreeSelectionChange);

	m_debugTree->addImage(new ui::StyleBitmap(L"Flash.MovieClip"), 1);
	m_debugTree->addImage(new ui::StyleBitmap(L"Flash.Shape"), 1);
	m_debugTree->addImage(new ui::StyleBitmap(L"Flash.MorphShape"), 1);
	m_debugTree->addImage(new ui::StyleBitmap(L"Flash.Text"), 1);
	m_debugTree->addImage(new ui::StyleBitmap(L"Flash.Edit"), 1);
	m_debugTree->addImage(new ui::StyleBitmap(L"Flash.Button"), 1);

	m_debugGrid = new ui::custom::GridView();
	m_debugGrid->create(splitter2, ui::WsDoubleBuffer);
	m_debugGrid->addColumn(new ui::custom::GridColumn(L"Name", ui::scaleBySystemDPI(150)));
	m_debugGrid->addColumn(new ui::custom::GridColumn(L"Value", ui::scaleBySystemDPI(300)));

	Ref< ui::Container > container = new ui::Container();
	container->create(splitter, ui::WsNone, new ui::TableLayout(L"100%", L"100%,*", 0, 0));

	m_debugView = new DebugView();
	m_debugView->create(container);
	m_debugView->addEventHandler< ui::PaintEvent >(this, &ClientPage::eventDebugViewPaint);

	m_statusBar = new ui::custom::StatusBar();
	m_statusBar->create(container, ui::WsDoubleBuffer);

	m_transport = transport;

	addEventHandler< ui::TimerEvent >(this, &ClientPage::eventTimer);
	startTimer(20);

	update();
	return true;
}

void ClientPage::updateSelection()
{
	RefArray< ui::custom::TreeViewItem > items;
	m_debugTree->getItems(items, ui::custom::TreeView::GfDescendants | ui::custom::TreeView::GfSelectedOnly);

	Ref< InstanceDebugInfo > debugInfo;
	if (items.size() == 1)
		debugInfo = items.front()->getData< InstanceDebugInfo >(L"DEBUGINFO");

	m_debugGrid->removeAllRows();
	if (debugInfo)
	{
		m_debugView->setHighlight(debugInfo);

		Vector2 lt, ls; float lr;
		debugInfo->getLocalTransform().decompose(&lt, &ls, &lr);

		Vector2 gt, gs; float gr;
		debugInfo->getGlobalTransform().decompose(&gt, &gs, &gr);

		Aabb2 lb = debugInfo->getBounds();
		Aabb2 gb = debugInfo->getGlobalTransform() * lb;

		auto cx = debugInfo->getColorTransform();
		
		StringOutputStream ss;
		Ref< ui::custom::GridRow > row;

		ss.reset();
		ss << mbstows(debugInfo->getName());

		row = new ui::custom::GridRow();
		row->add(new ui::custom::GridItem(L"Name"));
		row->add(new ui::custom::GridItem(ss.str()));
		m_debugGrid->addRow(row);

		if (const EditInstanceDebugInfo* editDebugInfo = dynamic_type_cast< const EditInstanceDebugInfo* >(debugInfo))
		{
			ss.reset();
			ss << editDebugInfo->getText();

			row = new ui::custom::GridRow();
			row->add(new ui::custom::GridItem(L"Text"));
			row->add(new ui::custom::GridItem(ss.str()));
			m_debugGrid->addRow(row);
		}

		ss.reset();
		ss << int(lt.x / 20.0f) << L", " << int(lt.y / 20.0f);

		row = new ui::custom::GridRow();
		row->add(new ui::custom::GridItem(L"Local position"));
		row->add(new ui::custom::GridItem(ss.str()));
		m_debugGrid->addRow(row);

		ss.reset();
		ss << ls.x << L", " << ls.y;

		row = new ui::custom::GridRow();
		row->add(new ui::custom::GridItem(L"Local scale"));
		row->add(new ui::custom::GridItem(ss.str()));
		m_debugGrid->addRow(row);

		ss.reset();
		ss << rad2deg(lr);

		row = new ui::custom::GridRow();
		row->add(new ui::custom::GridItem(L"Local rotatation"));
		row->add(new ui::custom::GridItem(ss.str()));
		m_debugGrid->addRow(row);

		ss.reset();
		ss << int(lb.mn.x / 20.0f) << L", " << int(lb.mn.y / 20.0f) << L" - " << int(lb.mx.x / 20.0f) << L", " << int(lb.mx.y / 20.0f);

		row = new ui::custom::GridRow();
		row->add(new ui::custom::GridItem(L"Local bounds"));
		row->add(new ui::custom::GridItem(ss.str()));
		m_debugGrid->addRow(row);

		ss.reset();
		ss << int(gt.x / 20.0f) << L", " << int(gt.y / 20.0f);

		row = new ui::custom::GridRow();
		row->add(new ui::custom::GridItem(L"Global position"));
		row->add(new ui::custom::GridItem(ss.str()));
		m_debugGrid->addRow(row);

		ss.reset();
		ss << gs.x << L", " << gs.y;

		row = new ui::custom::GridRow();
		row->add(new ui::custom::GridItem(L"Global scale"));
		row->add(new ui::custom::GridItem(ss.str()));
		m_debugGrid->addRow(row);

		ss.reset();
		ss << rad2deg(gr);

		row = new ui::custom::GridRow();
		row->add(new ui::custom::GridItem(L"Global rotatation"));
		row->add(new ui::custom::GridItem(ss.str()));
		m_debugGrid->addRow(row);

		ss.reset();
		ss << int(gb.mn.x / 20.0f) << L", " << int(gb.mn.y / 20.0f) << L" - " << int(gb.mx.x / 20.0f) << L", " << int(gb.mx.y / 20.0f);

		row = new ui::custom::GridRow();
		row->add(new ui::custom::GridItem(L"Global bounds"));
		row->add(new ui::custom::GridItem(ss.str()));
		m_debugGrid->addRow(row);		

		ss.reset();
		ss << L"+[" << cx.add << L"], *[" << cx.mul << L"]";

		row = new ui::custom::GridRow();
		row->add(new ui::custom::GridItem(L"Global color transform"));
		row->add(new ui::custom::GridItem(ss.str()));
		m_debugGrid->addRow(row);
	}
	else
		m_debugView->setHighlight(0);
}

void ClientPage::buildDebugTree(ui::custom::TreeViewItem* parent, const RefArray< InstanceDebugInfo >& instances)
{
	for (auto instance : instances)
	{
		StringOutputStream ss;
		int32_t image = -1;

		if (is_a< ButtonInstanceDebugInfo >(instance))
			image = 5;
		else if (is_a< EditInstanceDebugInfo >(instance))
			image = 4;
		else if (is_a< MorphShapeInstanceDebugInfo >(instance))
			image = 2;
		else if (is_a< ShapeInstanceDebugInfo >(instance))
			image = 1;
		else if (is_a< SpriteInstanceDebugInfo >(instance))
			image = 0;
		else if (is_a< TextInstanceDebugInfo >(instance))
			image= 3;

		if (!instance->getName().empty())
			ss << mbstows(instance->getName());
		else
			ss << L"<unnamed>";

		Ref< ui::custom::TreeViewItem > item = m_debugTree->createItem(parent, ss.str(), image);
		item->setData(L"DEBUGINFO", instance);

		if (SpriteInstanceDebugInfo* spriteInstance = dynamic_type_cast< SpriteInstanceDebugInfo* >(instance))
			buildDebugTree(item, spriteInstance->getChildrenDebugInfo());
	}
}

void ClientPage::eventToolBarClick(ui::custom::ToolBarButtonClickEvent* event)
{
	if (event->getCommand() == L"Traktor.Flash.Load")
	{
		ui::FileDialog fileDialog;
		fileDialog.create(this, L"Load capture...", L"All files;*.*", false);

		Path fileName;
		if (fileDialog.showModal(fileName) == ui::DrOk)
		{
			Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmRead);
			if (file)
			{
				m_debugInfo = xml::XmlDeserializer(file).readObject< PostFrameDebugInfo >();
				file->close();

				if (m_debugInfo)
				{
					auto state = m_debugTree->captureState();
					m_debugTree->removeAllItems();

					buildDebugTree(0, m_debugInfo->getInstances());

					m_debugTree->applyState(state);
					m_debugTree->update();

					updateSelection();

					m_debugView->setDebugInfo(m_debugInfo);
					m_debugView->update();				
				}
			}
		}

		fileDialog.destroy();
	}
	else if (event->getCommand() == L"Traktor.Flash.Save")
	{
		if (m_debugInfo)
		{
			ui::FileDialog fileDialog;
			fileDialog.create(this, L"Save capture as...", L"All files;*.*", true);

			Path fileName;
			if (fileDialog.showModal(fileName) == ui::DrOk)
			{
				Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmWrite);
				if (file)
				{
					xml::XmlSerializer(file).writeObject(m_debugInfo);
					file->close();
				}
			}

			fileDialog.destroy();
		}
	}
	else if (event->getCommand() == L"Traktor.Flash.CaptureSingle")
	{
		CaptureControl captureControl(1);
		m_transport->send(&captureControl);
	}
	else if (event->getCommand() == L"Traktor.Flash.ToggleShowSelectedOnly")
	{
		ui::custom::ToolBarButton* button = mandatory_non_null_type_cast< ui::custom::ToolBarButton* >(event->getItem());
		m_debugView->setHighlightOnly(button->isToggled());
		m_debugView->update();
	}
	else if (event->getCommand() == L"Traktor.Flash.ToggleShowOutline")
	{
		ui::custom::ToolBarButton* button = mandatory_non_null_type_cast< ui::custom::ToolBarButton* >(event->getItem());
		m_debugView->setOutline(button->isToggled());
		m_debugView->update();
	}
}

void ClientPage::eventDebugTreeSelectionChange(ui::SelectionChangeEvent* event)
{
	updateSelection();
	m_debugView->update();
}

void ClientPage::eventDebugViewPaint(ui::PaintEvent* event)
{
	StringOutputStream ss;
	ss << L"Mouse position: " << int32_t(m_debugView->getMousePosition().x / 20.0f) << L", " << int32_t(m_debugView->getMousePosition().y / 20.0f);
	m_statusBar->setText(ss.str());
}

void ClientPage::eventTimer(ui::TimerEvent* event)
{
	Ref< ISerializable > debugInfo;
	if (m_transport->recv(
		makeTypeInfoSet< MovieDebugInfo, PostFrameDebugInfo >(),
		10,
		debugInfo
	) != net::BidirectionalObjectTransport::RtSuccess)
		return;

	if (const PostFrameDebugInfo* postFrame = dynamic_type_cast< const PostFrameDebugInfo* >(debugInfo))
	{
		auto state = m_debugTree->captureState();
		m_debugTree->removeAllItems();

		buildDebugTree(0, postFrame->getInstances());

		m_debugTree->applyState(state);
		m_debugTree->update();

		updateSelection();

		m_debugView->setDebugInfo(postFrame);
		m_debugView->update();

		m_debugInfo = postFrame;
	}
	else if (const MovieDebugInfo* movieInfo = dynamic_type_cast< const MovieDebugInfo* >(debugInfo))
	{
		getParent()->setText(movieInfo->getName());
		getParent()->update();
	}
}

	}
}

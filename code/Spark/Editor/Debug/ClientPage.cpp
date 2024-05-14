/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Format.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Spark/Debug/ButtonInstanceDebugInfo.h"
#include "Spark/Debug/CaptureControl.h"
#include "Spark/Debug/EditInstanceDebugInfo.h"
#include "Spark/Debug/FrameDebugInfo.h"
#include "Spark/Debug/MorphShapeInstanceDebugInfo.h"
#include "Spark/Debug/MovieDebugInfo.h"
#include "Spark/Debug/ShapeInstanceDebugInfo.h"
#include "Spark/Debug/SpriteInstanceDebugInfo.h"
#include "Spark/Debug/TextInstanceDebugInfo.h"
#include "Spark/Editor/Debug/ClientPage.h"
#include "Spark/Editor/Debug/DebugView.h"
#include "Ui/Application.h"
#include "Ui/Edit.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/FileDialog.h"
#include "Ui/Splitter.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridView.h"
#include "Ui/StatusBar/StatusBar.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarEmbed.h"
#include "Ui/ToolBar/ToolBarSeparator.h"
#include "Ui/TreeView/TreeView.h"
#include "Ui/TreeView/TreeViewItem.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

namespace traktor
{
	namespace spark
	{

bool ClientPage::create(ui::Widget* parent, net::BidirectionalObjectTransport* transport)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0_ut, 0_ut)))
		return false;

	m_toolBar = new ui::ToolBar();
	m_toolBar->create(this);

	m_toolBar->addImage(new ui::StyleBitmap(L"Flash.PreviousFrame"));
	m_toolBar->addImage(new ui::StyleBitmap(L"Flash.NextFrame"));

	m_toolBar->addItem(new ui::ToolBarButton(L"Load", ui::Command(L"Traktor.Flash.Load")));
	m_toolBar->addItem(new ui::ToolBarButton(L"Save", ui::Command(L"Traktor.Flash.Save")));
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(new ui::ToolBarButton(L"Capture", ui::Command(L"Traktor.Flash.CaptureSingle")));
	m_toolBar->addItem(new ui::ToolBarButton(L"Previous Frame", 0, ui::Command(L"Traktor.Flash.PreviousFrame")));

	m_editFrame = new ui::Edit();
	m_editFrame->create(m_toolBar, L"0", ui::WsNone, new ui::NumericEditValidator(false, 0));
	m_editFrame->addEventHandler< ui::ContentChangeEvent >(this, &ClientPage::eventFrameChange);
	m_toolBar->addItem(new ui::ToolBarEmbed(m_editFrame, 30_ut));

	m_toolBar->addItem(new ui::ToolBarButton(L"Next Frame", 1, ui::Command(L"Traktor.Flash.NextFrame")));
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(new ui::ToolBarButton(L"Selected Only", ui::Command(L"Traktor.Flash.ToggleShowSelectedOnly"), ui::ToolBarButton::BsText | ui::ToolBarButton::BsToggle));
	m_toolBar->addItem(new ui::ToolBarButton(L"Visible Only", ui::Command(L"Traktor.Flash.ToggleShowVisibleOnly"), ui::ToolBarButton::BsText | ui::ToolBarButton::BsToggled));
	m_toolBar->addItem(new ui::ToolBarButton(L"Outline", ui::Command(L"Traktor.Flash.ToggleShowOutline"), ui::ToolBarButton::BsText | ui::ToolBarButton::BsToggled));
	m_toolBar->addItem(new ui::ToolBarButton(L"Masks", ui::Command(L"Traktor.Flash.ToggleShowMasks"), ui::ToolBarButton::BsText | ui::ToolBarButton::BsToggle));

	m_toolBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &ClientPage::eventToolBarClick);

	Ref< ui::Splitter > splitter = new ui::Splitter();
	splitter->create(this, true, 30_ut, true);

	Ref< ui::Splitter > splitter2 = new ui::Splitter();
	splitter2->create(splitter, false, 50_ut, true);

	m_debugTree = new ui::TreeView();
	m_debugTree->create(splitter2, ui::TreeView::WsTreeButtons | ui::TreeView::WsTreeLines | ui::WsAccelerated);
	m_debugTree->addEventHandler< ui::SelectionChangeEvent >(this, &ClientPage::eventDebugTreeSelectionChange);

	m_debugTree->addImage(new ui::StyleBitmap(L"Flash.MovieClip"));
	m_debugTree->addImage(new ui::StyleBitmap(L"Flash.Shape"));
	m_debugTree->addImage(new ui::StyleBitmap(L"Flash.MorphShape"));
	m_debugTree->addImage(new ui::StyleBitmap(L"Flash.Text"));
	m_debugTree->addImage(new ui::StyleBitmap(L"Flash.Edit"));
	m_debugTree->addImage(new ui::StyleBitmap(L"Flash.Button"));

	m_debugGrid = new ui::GridView();
	m_debugGrid->create(splitter2, ui::WsDoubleBuffer);
	m_debugGrid->addColumn(new ui::GridColumn(L"Name", 150_ut));
	m_debugGrid->addColumn(new ui::GridColumn(L"Value", 300_ut));

	Ref< ui::Container > container = new ui::Container();
	container->create(splitter, ui::WsNone, new ui::TableLayout(L"100%", L"100%,*", 0_ut, 0_ut));

	m_debugView = new DebugView();
	m_debugView->create(container);
	m_debugView->addEventHandler< ui::PaintEvent >(this, &ClientPage::eventDebugViewPaint);

	m_statusBar = new ui::StatusBar();
	m_statusBar->create(container, ui::WsDoubleBuffer);
	m_statusBar->addColumn(-1);

	m_transport = transport;
	m_selectedFrame = -1;

	addEventHandler< ui::TimerEvent >(this, &ClientPage::eventTimer);
	startTimer(20);

	update();

	// Capture initial frame.
	CaptureControl captureControl(1);
	m_transport->send(&captureControl);

	return true;
}

void ClientPage::updateSelection()
{
	RefArray< ui::TreeViewItem > items;
	m_debugTree->getItems(items, ui::TreeView::GfDescendants | ui::TreeView::GfSelectedOnly);

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
		Ref< ui::GridRow > row;

		ss.reset();
		ss << mbstows(debugInfo->getName());

		row = new ui::GridRow();
		row->add(L"Name");
		row->add(ss.str());
		m_debugGrid->addRow(row);

		if (const SpriteInstanceDebugInfo* spriteDebugInfo = dynamic_type_cast< const SpriteInstanceDebugInfo* >(debugInfo))
		{
			ss.reset();
			ss << mbstows(spriteDebugInfo->getClassName());

			row = new ui::GridRow();
			row->add(L"Class");
			row->add(ss.str());
			m_debugGrid->addRow(row);

			row = new ui::GridRow();
			row->add(L"Mask");
			row->add(spriteDebugInfo->getMask() ? L"Yes" : L"No");
			m_debugGrid->addRow(row);

			row = new ui::GridRow();
			row->add(L"Clipped");
			row->add(spriteDebugInfo->getClipped() ? L"Yes" : L"No");
			m_debugGrid->addRow(row);

			row = new ui::GridRow();
			row->add(L"9Grid Scaling");
			row->add(!spriteDebugInfo->getScalingGrid().empty() ? L"Yes" : L"No");
			m_debugGrid->addRow(row);
		}

		if (const EditInstanceDebugInfo* editDebugInfo = dynamic_type_cast< const EditInstanceDebugInfo* >(debugInfo))
		{
			ss.reset();
			ss << editDebugInfo->getText();

			row = new ui::GridRow();
			row->add(L"Text");
			row->add(ss.str());
			m_debugGrid->addRow(row);

			ss.reset();
			ss << editDebugInfo->getTextColor().getRed() << L", " << editDebugInfo->getTextColor().getGreen() << L", " << editDebugInfo->getTextColor().getBlue() << L", " << editDebugInfo->getTextColor().getAlpha();

			row = new ui::GridRow();
			row->add(L"Text Color");
			row->add(ss.str());
			m_debugGrid->addRow(row);
		}

		row = new ui::GridRow();
		row->add(L"Visible");
		row->add(debugInfo->getVisible() ? L"Yes" : L"No");
		m_debugGrid->addRow(row);


		auto lm = debugInfo->getLocalTransform();

		ss.reset();
		ss << lm.e11 << L", " << lm.e12 << L", " << lm.e13;

		row = new ui::GridRow();
		row->add(L"Local transform [11,12,13]");
		row->add(ss.str());
		m_debugGrid->addRow(row);

		ss.reset();
		ss << lm.e21 << L", " << lm.e22 << L", " << lm.e23;

		row = new ui::GridRow();
		row->add(L"Local transform [21,22,23]");
		row->add(ss.str());
		m_debugGrid->addRow(row);

		ss.reset();
		ss << lm.e31 << L", " << lm.e32 << L", " << lm.e33;

		row = new ui::GridRow();
		row->add(L"Local transform [31,32,33]");
		row->add(ss.str());
		m_debugGrid->addRow(row);



		ss.reset();
		ss << int(lt.x / 20.0f) << L", " << int(lt.y / 20.0f);

		row = new ui::GridRow();
		row->add(L"Local position");
		row->add(ss.str());
		m_debugGrid->addRow(row);

		ss.reset();
		ss << ls.x << L", " << ls.y;

		row = new ui::GridRow();
		row->add(L"Local scale");
		row->add(ss.str());
		m_debugGrid->addRow(row);

		ss.reset();
		ss << rad2deg(lr);

		row = new ui::GridRow();
		row->add(L"Local rotatation");
		row->add(ss.str());
		m_debugGrid->addRow(row);

		ss.reset();
		ss << int(lb.mn.x / 20.0f) << L", " << int(lb.mn.y / 20.0f) << L" - " << int(lb.mx.x / 20.0f) << L", " << int(lb.mx.y / 20.0f);

		row = new ui::GridRow();
		row->add(L"Local bounds");
		row->add(ss.str());
		m_debugGrid->addRow(row);

		ss.reset();
		ss << int(gt.x / 20.0f) << L", " << int(gt.y / 20.0f);

		row = new ui::GridRow();
		row->add(L"Global position");
		row->add(ss.str());
		m_debugGrid->addRow(row);

		ss.reset();
		ss << gs.x << L", " << gs.y;

		row = new ui::GridRow();
		row->add(L"Global scale");
		row->add(ss.str());
		m_debugGrid->addRow(row);

		ss.reset();
		ss << rad2deg(gr);

		row = new ui::GridRow();
		row->add(L"Global rotatation");
		row->add(ss.str());
		m_debugGrid->addRow(row);

		ss.reset();
		ss << int(gb.mn.x / 20.0f) << L", " << int(gb.mn.y / 20.0f) << L" - " << int(gb.mx.x / 20.0f) << L", " << int(gb.mx.y / 20.0f);

		row = new ui::GridRow();
		row->add(L"Global bounds");
		row->add(ss.str());
		m_debugGrid->addRow(row);

		ss.reset();
		ss << L"+[" << cx.add << L"], *[" << cx.mul << L"]";

		row = new ui::GridRow();
		row->add(L"Global color transform");
		row->add(ss.str());
		m_debugGrid->addRow(row);
	}
	else
		m_debugView->setHighlight(0);
}

void ClientPage::buildDebugTree(ui::TreeViewItem* parent, const RefArray< InstanceDebugInfo >& instances)
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

		Ref< ui::TreeViewItem > item = m_debugTree->createItem(parent, ss.str(), 1);
		item->setImage(0, image);
		item->setData(L"DEBUGINFO", instance);

		if (SpriteInstanceDebugInfo* spriteInstance = dynamic_type_cast< SpriteInstanceDebugInfo* >(instance))
			buildDebugTree(item, spriteInstance->getChildrenDebugInfo());
	}
}

void ClientPage::selectFrame(int32_t frame)
{
	frame = clamp< int32_t >(frame, 0, int32_t(m_frameInfos.size() - 1));
	if (frame == m_selectedFrame)
		return;

	const FrameDebugInfo* frameInfo = m_frameInfos[frame];

	auto state = m_debugTree->captureState();
	m_debugTree->removeAllItems();

	if (frameInfo)
		buildDebugTree(0, frameInfo->getInstances());

	m_debugTree->applyState(state);
	m_debugTree->update();

	updateSelection();

	m_debugView->setDebugInfo(frameInfo);
	m_debugView->update();

	m_selectedFrame = frame;
	m_editFrame->setText(toString< int32_t >(frame));
}

void ClientPage::eventFrameChange(ui::ContentChangeEvent* event)
{
	selectFrame(parseString< int32_t >(m_editFrame->getText()));
}

void ClientPage::eventToolBarClick(ui::ToolBarButtonClickEvent* event)
{
	if (event->getCommand() == L"Traktor.Flash.Load")
	{
		ui::FileDialog fileDialog;
		fileDialog.create(this, type_name(this), L"Load capture...", L"All files;*.*", L"", false);

		Path fileName;
		if (fileDialog.showModal(fileName) == ui::DialogResult::Ok)
		{
			Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmRead);
			if (file)
			{
				//m_debugInfo = xml::XmlDeserializer(file).readObject< FrameDebugInfo >();
				//file->close();

				//if (m_debugInfo)
				//{
				//	auto state = m_debugTree->captureState();
				//	m_debugTree->removeAllItems();

				//	buildDebugTree(0, m_debugInfo->getInstances());

				//	m_debugTree->applyState(state);
				//	m_debugTree->update();

				//	updateSelection();

				//	m_debugView->setDebugInfo(m_debugInfo);
				//	m_debugView->update();
				//}
			}
		}

		fileDialog.destroy();
	}
	else if (event->getCommand() == L"Traktor.Flash.Save")
	{
		//if (m_debugInfo)
		//{
		//	ui::FileDialog fileDialog;
		//	fileDialog.create(this, L"Save capture as...", L"All files;*.*", true);

		//	Path fileName;
		//	if (fileDialog.showModal(fileName) == ui::DialogResult::Ok)
		//	{
		//		Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmWrite);
		//		if (file)
		//		{
		//			xml::XmlSerializer(file).writeObject(m_debugInfo);
		//			file->close();
		//		}
		//	}

		//	fileDialog.destroy();
		//}
	}
	else if (event->getCommand() == L"Traktor.Flash.CaptureSingle")
	{
		CaptureControl captureControl(1);
		m_transport->send(&captureControl);
	}
	else if (event->getCommand() == L"Traktor.Flash.PreviousFrame")
	{
		if (m_selectedFrame > 0)
			selectFrame(m_selectedFrame - 1);
	}
	else if (event->getCommand() == L"Traktor.Flash.NextFrame")
	{
		if (m_selectedFrame < int32_t(m_frameInfos.size() - 1))
			selectFrame(m_selectedFrame + 1);
	}
	else if (event->getCommand() == L"Traktor.Flash.ToggleShowSelectedOnly")
	{
		ui::ToolBarButton* button = mandatory_non_null_type_cast< ui::ToolBarButton* >(event->getItem());
		m_debugView->setHighlightOnly(button->isToggled());
		m_debugView->update();
	}
	else if (event->getCommand() == L"Traktor.Flash.ToggleShowVisibleOnly")
	{
		ui::ToolBarButton* button = mandatory_non_null_type_cast< ui::ToolBarButton* >(event->getItem());
		m_debugView->setVisibleOnly(button->isToggled());
		m_debugView->update();
	}
	else if (event->getCommand() == L"Traktor.Flash.ToggleShowOutline")
	{
		ui::ToolBarButton* button = mandatory_non_null_type_cast< ui::ToolBarButton* >(event->getItem());
		m_debugView->setOutline(button->isToggled());
		m_debugView->update();
	}
	else if (event->getCommand() == L"Traktor.Flash.ToggleShowMasks")
	{
		ui::ToolBarButton* button = mandatory_non_null_type_cast< ui::ToolBarButton* >(event->getItem());
		m_debugView->setShowMasks(button->isToggled());
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
	m_statusBar->setText(0, ss.str());
}

void ClientPage::eventTimer(ui::TimerEvent* event)
{
	Ref< ISerializable > debugInfo;
	if (m_transport->recv(
		makeTypeInfoSet< MovieDebugInfo, FrameDebugInfo >(),
		10,
		debugInfo
	) != net::BidirectionalObjectTransport::Result::Success)
		return;

	if (const FrameDebugInfo* frameInfo = dynamic_type_cast< const FrameDebugInfo* >(debugInfo))
	{
		m_frameInfos.push_back(frameInfo);
		selectFrame(int32_t(m_frameInfos.size() - 1));
	}
	else if (const MovieDebugInfo* movieInfo = dynamic_type_cast< const MovieDebugInfo* >(debugInfo))
	{
		getParent()->setText(movieInfo->getName());
		getParent()->update();
	}
}

	}
}

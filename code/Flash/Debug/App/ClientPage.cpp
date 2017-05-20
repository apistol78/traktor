/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
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
#include "Ui/TableLayout.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"
#include "Ui/Custom/TreeView/TreeView.h"
#include "Ui/Custom/TreeView/TreeViewItem.h"

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

	m_toolBar->addItem(new ui::custom::ToolBarButton(L"Stop", ui::Command(L"Traktor.Flash.CaptureStop")));
	m_toolBar->addItem(new ui::custom::ToolBarButton(L"Single", ui::Command(L"Traktor.Flash.CaptureSingle")));
	m_toolBar->addItem(new ui::custom::ToolBarButton(L"Continuous", ui::Command(L"Traktor.Flash.CaptureContinuous")));
	m_toolBar->addItem(new ui::custom::ToolBarSeparator());
	m_toolBar->addItem(new ui::custom::ToolBarButton(L"Selected Only", ui::Command(L"Traktor.Flash.ToggleShowSelectedOnly"), ui::custom::ToolBarButton::BsText | ui::custom::ToolBarButton::BsToggle));

	m_toolBar->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &ClientPage::eventToolBarClick);

	Ref< ui::custom::Splitter > splitter = new ui::custom::Splitter();
	splitter->create(this, true, 20, true);

	m_debugTree = new ui::custom::TreeView();
	m_debugTree->create(splitter, ui::custom::TreeView::WsTreeButtons | ui::custom::TreeView::WsTreeLines | ui::WsAccelerated);
	m_debugTree->addEventHandler< ui::SelectionChangeEvent >(this, &ClientPage::eventDebugTreeSelectionChange);

	m_debugView = new DebugView();
	m_debugView->create(splitter);

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
	if (items.size() == 1)
		m_debugView->setHighlight(items.front()->getData< InstanceDebugInfo >(L"DEBUGINFO"));
	else
		m_debugView->setHighlight(0);
}

void ClientPage::buildDebugTree(ui::custom::TreeViewItem* parent, const RefArray< InstanceDebugInfo >& instances)
{
	for (auto instance : instances)
	{
		StringOutputStream ss;

		if (is_a< ButtonInstanceDebugInfo >(instance))
			ss << L"BT: ";
		else if (is_a< EditInstanceDebugInfo >(instance))
			ss << L"ED: ";
		else if (is_a< MorphShapeInstanceDebugInfo >(instance))
			ss << L"MO: ";
		else if (is_a< ShapeInstanceDebugInfo >(instance))
			ss << L"SH: ";
		else if (is_a< SpriteInstanceDebugInfo >(instance))
			ss << L"MC: ";
		else if (is_a< TextInstanceDebugInfo >(instance))
			ss << L"TX: ";
		else
			ss << L"??: ";

		if (!instance->getName().empty())
			ss << mbstows(instance->getName());
		else
			ss << L"<unnamed>";

		Ref< ui::custom::TreeViewItem > item = m_debugTree->createItem(parent, ss.str());
		item->setData(L"DEBUGINFO", instance);

		if (SpriteInstanceDebugInfo* spriteInstance = dynamic_type_cast< SpriteInstanceDebugInfo* >(instance))
			buildDebugTree(item, spriteInstance->getChildrenDebugInfo());
	}
}

void ClientPage::eventToolBarClick(ui::custom::ToolBarButtonClickEvent* event)
{
	if (event->getCommand() == L"Traktor.Flash.CaptureStop")
	{
		CaptureControl captureControl(CaptureControl::MdStop);
		m_transport->send(&captureControl);
	}
	else if (event->getCommand() == L"Traktor.Flash.CaptureSingle")
	{
		CaptureControl captureControl(CaptureControl::MdSingle);
		m_transport->send(&captureControl);
	}
	else if (event->getCommand() == L"Traktor.Flash.CaptureContinuous")
	{
		CaptureControl captureControl(CaptureControl::MdContinuous);
		m_transport->send(&captureControl);
	}
	else if (event->getCommand() == L"Traktor.Flash.ToggleShowSelectedOnly")
	{
		ui::custom::ToolBarButton* button = mandatory_non_null_type_cast< ui::custom::ToolBarButton* >(event->getItem());
		m_debugView->setHighlightOnly(button->isToggled());
		m_debugView->update();
	}
}

void ClientPage::eventDebugTreeSelectionChange(ui::SelectionChangeEvent* event)
{
	updateSelection();
	m_debugView->update();
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
	}
	else if (const MovieDebugInfo* movieInfo = dynamic_type_cast< const MovieDebugInfo* >(debugInfo))
	{
		log::info << L"Received movie debug info" << Endl;

		getParent()->setText(movieInfo->getName());
	}
}

	}
}

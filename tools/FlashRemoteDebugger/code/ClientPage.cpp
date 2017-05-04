/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Io/StringOutputStream.h>
#include <Core/Misc/TString.h>
#include <Flash/Debug/ButtonInstanceDebugInfo.h>
#include <Flash/Debug/EditInstanceDebugInfo.h>
#include <Flash/Debug/MorphShapeInstanceDebugInfo.h>
#include <Flash/Debug/ShapeInstanceDebugInfo.h>
#include <Flash/Debug/SpriteInstanceDebugInfo.h>
#include <Flash/Debug/TextInstanceDebugInfo.h>
#include <Ui/FloodLayout.h>
#include <Ui/Custom/Splitter.h>
#include "ClientPage.h"
#include "DebugView.h"

using namespace traktor;

bool ClientPage::create(ui::Widget* parent, net::BidirectionalObjectTransport* transport)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::FloodLayout()))
		return false;

	Ref< ui::custom::Splitter > splitter = new ui::custom::Splitter();
	splitter->create(this, true, 30, true);

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

void ClientPage::buildDebugTree(ui::custom::TreeViewItem* parent, const RefArray< flash::InstanceDebugInfo >& instances)
{
	for (auto instance : instances)
	{
		StringOutputStream ss;

		if (is_a< flash::ButtonInstanceDebugInfo >(instance))
			ss << L"BT: ";
		else if (is_a< flash::EditInstanceDebugInfo >(instance))
			ss << L"ED: ";
		else if (is_a< flash::MorphShapeInstanceDebugInfo >(instance))
			ss << L"MO: ";
		else if (is_a< flash::ShapeInstanceDebugInfo >(instance))
			ss << L"SH: ";
		else if (is_a< flash::SpriteInstanceDebugInfo >(instance))
			ss << L"MC: ";
		else if (is_a< flash::TextInstanceDebugInfo >(instance))
			ss << L"TX: ";
		else
			ss << L"??: ";

		if (!instance->getName().empty())
			ss << mbstows(instance->getName());
		else
			ss << L"<unnamed>";

		Ref< ui::custom::TreeViewItem > item = m_debugTree->createItem(parent, ss.str());
		item->setData(L"DEBUGINFO", instance);

		if (flash::SpriteInstanceDebugInfo* spriteInstance = dynamic_type_cast< flash::SpriteInstanceDebugInfo* >(instance))
			buildDebugTree(item, spriteInstance->getChildrenDebugInfo());
	}
}

void ClientPage::eventDebugTreeSelectionChange(ui::SelectionChangeEvent* event)
{
	//RefArray< ui::custom::TreeViewItem > items;
	//m_debugTree->getItems(items, ui::custom::TreeView::GfDescendants | ui::custom::TreeView::GfSelectedOnly);
	//if (items.size() == 1)
	//	m_debugView->highlight
}

void ClientPage::eventTimer(ui::TimerEvent* event)
{
	Ref< traktor::flash::PostFrameDebugInfo > debugInfo;
	if (m_transport->recv(10, debugInfo) != net::BidirectionalObjectTransport::RtSuccess)
		return;

	m_transport->flush< flash::PostFrameDebugInfo >();

	auto state = m_debugTree->captureState();
	m_debugTree->removeAllItems();

	buildDebugTree(0, debugInfo->getInstances());

	m_debugTree->applyState(state);
	m_debugTree->update();

	m_debugView->setDebugInfo(debugInfo);
}

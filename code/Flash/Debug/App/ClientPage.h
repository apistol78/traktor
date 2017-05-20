/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_ClientPage_H
#define traktor_flash_ClientPage_H

#include "Flash/Debug/PostFrameDebugInfo.h"
#include "Net/BidirectionalObjectTransport.h"
#include "Ui/Container.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
		
class ToolBar;
class ToolBarButtonClickEvent;
class TreeView;
class TreeViewItem;
		
		}
	}

	namespace flash
	{

class DebugView;

class ClientPage : public ui::Container
{
public:
	bool create(
		ui::Widget* parent,
		net::BidirectionalObjectTransport* transport
	);

private:
	Ref< net::BidirectionalObjectTransport > m_transport;
	Ref< ui::custom::ToolBar > m_toolBar;
	Ref< ui::custom::TreeView > m_debugTree;
	Ref< DebugView > m_debugView;
	Ref< PostFrameDebugInfo > m_debugInfo;

	void updateSelection();

	void buildDebugTree(ui::custom::TreeViewItem* parent, const RefArray< InstanceDebugInfo >& instances);

	void eventToolBarClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventDebugTreeSelectionChange(ui::SelectionChangeEvent* event);

	void eventTimer(ui::TimerEvent* event);
};

	}
}

#endif	// traktor_flash_ClientPage_H

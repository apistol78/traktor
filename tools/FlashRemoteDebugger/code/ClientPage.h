/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef ClientPage_H
#define ClientPage_H

#include <Flash/Debug/PostFrameDebugInfo.h>
#include <Net/BidirectionalObjectTransport.h>
#include <Ui/Container.h>
#include <Ui/Custom/TreeView/TreeView.h>
#include <Ui/Custom/TreeView/TreeViewItem.h>

class DebugView;

class ClientPage : public traktor::ui::Container
{
public:
	bool create(
		traktor::ui::Widget* parent,
		traktor::net::BidirectionalObjectTransport* transport
	);

private:
	traktor::Ref< traktor::net::BidirectionalObjectTransport > m_transport;
	traktor::Ref< traktor::ui::custom::TreeView > m_debugTree;
	traktor::Ref< DebugView > m_debugView;
	traktor::Ref< traktor::flash::PostFrameDebugInfo > m_debugInfo;

	void buildDebugTree(traktor::ui::custom::TreeViewItem* parent, const traktor::RefArray< traktor::flash::InstanceDebugInfo >& instances);

	void eventDebugTreeSelectionChange(traktor::ui::SelectionChangeEvent* event);

	void eventTimer(traktor::ui::TimerEvent* event);
};

#endif	// ClientPage_H

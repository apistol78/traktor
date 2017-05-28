/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_ClientPage_H
#define traktor_flash_ClientPage_H

#include "Net/BidirectionalObjectTransport.h"
#include "Ui/Container.h"

namespace traktor
{
	namespace ui
	{

class Edit;

		namespace custom
		{
		
class GridView;
class StatusBar;
class ToolBar;
class ToolBarButtonClickEvent;
class TreeView;
class TreeViewItem;
		
		}
	}

	namespace flash
	{

class DebugView;
class FrameDebugInfo;
class InstanceDebugInfo;

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
	Ref< ui::Edit > m_editFrame;
	Ref< ui::custom::TreeView > m_debugTree;
	Ref< ui::custom::GridView > m_debugGrid;
	Ref< DebugView > m_debugView;
	Ref< ui::custom::StatusBar > m_statusBar;
	RefArray< const FrameDebugInfo > m_frameInfos;
	int32_t m_selectedFrame;

	void updateSelection();

	void selectFrame(int32_t frame);

	void buildDebugTree(ui::custom::TreeViewItem* parent, const RefArray< InstanceDebugInfo >& instances);

	void eventFrameChange(ui::ContentChangeEvent* event);

	void eventToolBarClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventDebugTreeSelectionChange(ui::SelectionChangeEvent* event);

	void eventDebugViewPaint(ui::PaintEvent* event);

	void eventTimer(ui::TimerEvent* event);
};

	}
}

#endif	// traktor_flash_ClientPage_H

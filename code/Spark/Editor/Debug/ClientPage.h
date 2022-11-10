/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Net/BidirectionalObjectTransport.h"
#include "Ui/Container.h"

namespace traktor
{
	namespace ui
	{

class Edit;
class GridView;
class StatusBar;
class ToolBar;
class ToolBarButtonClickEvent;
class TreeView;
class TreeViewItem;

	}

	namespace spark
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
	Ref< ui::ToolBar > m_toolBar;
	Ref< ui::Edit > m_editFrame;
	Ref< ui::TreeView > m_debugTree;
	Ref< ui::GridView > m_debugGrid;
	Ref< DebugView > m_debugView;
	Ref< ui::StatusBar > m_statusBar;
	RefArray< const FrameDebugInfo > m_frameInfos;
	int32_t m_selectedFrame;

	void updateSelection();

	void selectFrame(int32_t frame);

	void buildDebugTree(ui::TreeViewItem* parent, const RefArray< InstanceDebugInfo >& instances);

	void eventFrameChange(ui::ContentChangeEvent* event);

	void eventToolBarClick(ui::ToolBarButtonClickEvent* event);

	void eventDebugTreeSelectionChange(ui::SelectionChangeEvent* event);

	void eventDebugViewPaint(ui::PaintEvent* event);

	void eventTimer(ui::TimerEvent* event);
};

	}
}


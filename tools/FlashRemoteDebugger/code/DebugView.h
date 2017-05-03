/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef DebugView_H
#define DebugView_H

#include <Flash/Debug/PostFrameDebugInfo.h>
#include <Net/BidirectionalObjectTransport.h>
#include <Ui/Widget.h>

class DebugView : public traktor::ui::Widget
{
public:
	bool create(
		traktor::ui::Widget* parent,
		traktor::net::BidirectionalObjectTransport* transport
	);

private:
	traktor::Ref< traktor::net::BidirectionalObjectTransport > m_transport;
	traktor::Ref< traktor::flash::PostFrameDebugInfo > m_debugInfo;

	void eventPaint(traktor::ui::PaintEvent* event);

	void eventTimer(traktor::ui::TimerEvent* event);
};

#endif	// DebugView_H


/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef DebugView_H
#define DebugView_H

#include <Flash/Debug/PostFrameDebugInfo.h>
#include <Ui/Widget.h>

class DebugView : public traktor::ui::Widget
{
public:
	bool create(
		traktor::ui::Widget* parent
	);

	void setDebugInfo(const traktor::flash::PostFrameDebugInfo* debugInfo);

private:
	traktor::Ref< const traktor::flash::PostFrameDebugInfo > m_debugInfo;
	traktor::ui::Point m_offset;
	traktor::ui::Point m_mouseLast;
	int32_t m_counter;

	void eventPaint(traktor::ui::PaintEvent* event);

	void eventMouseDown(traktor::ui::MouseButtonDownEvent* event);

	void eventMouseUp(traktor::ui::MouseButtonUpEvent* event);

	void eventMouseMove(traktor::ui::MouseMoveEvent* event);
};

#endif	// DebugView_H


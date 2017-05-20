/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_DebugView_H
#define traktor_flash_DebugView_H

#include "Flash/Debug/InstanceDebugInfo.h"
#include "Flash/Debug/PostFrameDebugInfo.h"
#include "Ui/Widget.h"

namespace traktor
{
	namespace flash
	{

class DebugView : public ui::Widget
{
public:
	bool create(
		ui::Widget* parent
	);

	void setDebugInfo(const PostFrameDebugInfo* debugInfo);

	void setHighlight(const InstanceDebugInfo* instance);

	void setHighlightOnly(bool highlightOnly);

private:
	Ref< const PostFrameDebugInfo > m_debugInfo;
	Ref< const InstanceDebugInfo > m_highlightInstance;
	bool m_highlightOnly;
	ui::Point m_offset;
	ui::Point m_mouseLast;
	int32_t m_counter;
	float m_scale;

	void eventPaint(ui::PaintEvent* event);

	void eventMouseDown(ui::MouseButtonDownEvent* event);

	void eventMouseUp(ui::MouseButtonUpEvent* event);

	void eventMouseMove(ui::MouseMoveEvent* event);

	void eventMouseWheel(ui::MouseWheelEvent* event);
};

	}
}

#endif	// traktor_flash_DebugView_H

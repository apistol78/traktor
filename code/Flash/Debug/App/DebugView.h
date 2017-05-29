/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_DebugView_H
#define traktor_flash_DebugView_H

#include "Flash/Debug/FrameDebugInfo.h"
#include "Flash/Debug/InstanceDebugInfo.h"
#include "Ui/Widget.h"

namespace traktor
{
	namespace drawing
	{

class Image;

	}

	namespace ui
	{
	
class Bitmap;
class StyleBitmap;

	}

	namespace flash
	{

class DebugView : public ui::Widget
{
public:
	struct ShapeCache
	{
		Ref< drawing::Image > image;
		Ref< ui::Bitmap > bitmap;
	};

	bool create(
		ui::Widget* parent
	);

	void setDebugInfo(const FrameDebugInfo* debugInfo);

	void setHighlight(const InstanceDebugInfo* instance);

	void setHighlightOnly(bool highlightOnly);

	void setVisibleOnly(bool visibleOnly);

	void setOutline(bool outline);

	void setShowMasks(bool showMasks);

	const Vector2& getMousePosition() const;

private:
	Ref< const FrameDebugInfo > m_debugInfo;
	Ref< const InstanceDebugInfo > m_highlightInstance;
	Ref< ui::StyleBitmap > m_bitmapPivot;
	bool m_highlightOnly;
	bool m_visibleOnly;
	bool m_outline;
	bool m_showMasks;
	ui::Point m_offset;
	ui::Point m_mouseLast;
	float m_scale;
	Vector2 m_mousePosition;
	std::map< void*, ShapeCache > m_shapeCache;

	void eventPaint(ui::PaintEvent* event);

	void eventMouseDown(ui::MouseButtonDownEvent* event);

	void eventMouseUp(ui::MouseButtonUpEvent* event);

	void eventMouseMove(ui::MouseMoveEvent* event);

	void eventMouseWheel(ui::MouseWheelEvent* event);
};

	}
}

#endif	// traktor_flash_DebugView_H

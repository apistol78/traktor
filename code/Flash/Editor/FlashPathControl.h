#ifndef traktor_flash_FlashPathControl_H
#define traktor_flash_FlashPathControl_H

#include "Flash/Path.h"
#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class FlashPathControl : public ui::Widget
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent, int style);

	void setPath(const Path& path);

private:
	Path m_path;

	Vector2 m_viewOffset;
	float m_viewScale;

	ui::Point m_lastMousePosition;

	std::vector< uint16_t > m_fillStyles;
	int m_fillStyleIndex;

	void eventPaint(ui::PaintEvent* event);

	void eventMouseButtonDown(ui::MouseButtonDownEvent* event);

	void eventMouseButtonUp(ui::MouseButtonUpEvent* event);

	void eventMouseMove(ui::MouseMoveEvent* event);

	void eventMouseWheel(ui::MouseWheelEvent* event);

	void eventKeyDown(ui::KeyDownEvent* event);
};

	}
}

#endif	// traktor_flash_FlashPathControl_H

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

	void eventPaint(ui::PaintEvent* event);
};

	}
}

#endif	// traktor_flash_FlashPathControl_H

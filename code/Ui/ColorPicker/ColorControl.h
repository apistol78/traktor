#pragma once

#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace drawing
	{

class Image;

	}

	namespace ui
	{

class Bitmap;

/*! \brief Color static control.
 * \ingroup UI
 */
class T_DLLCLASS ColorControl : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, int style);

	void setColor(const Color4ub& color);

	Color4ub getColor() const;

	virtual Size getPreferedSize() const override;

private:
	Color4ub m_color;
	Ref< drawing::Image > m_previewImage;
	Ref< Bitmap > m_previewBitmap;

	void eventPaint(PaintEvent* event);
};

	}
}


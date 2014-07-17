#ifndef traktor_ui_custom_GradientStatic_H
#define traktor_ui_custom_GradientStatic_H

#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Bitmap;

		namespace custom
		{

/*! \brief Gradient static control.
 * \ingroup UIC
 */
class T_DLLCLASS GradientStatic : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, const Color4ub& colorLeft, const Color4ub& colorRight, const Color4ub& colorText, const std::wstring& text, int style = WsNone);

	virtual Size getPreferedSize() const;

	void setColorLeft(const Color4ub& colorLeft);

	void setColorRight(const Color4ub& colorRight);

	void setColorText(const Color4ub& colorText);

private:
	Color4ub m_colorLeft;
	Color4ub m_colorRight;
	Color4ub m_colorText;

	void eventPaint(PaintEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_GradientStatic_H

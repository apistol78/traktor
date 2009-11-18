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
	bool create(Widget* parent, const Color& colorLeft, const Color& colorRight, const Color& colorText, const std::wstring& text, int style = WsNone);

	virtual Size getPreferedSize() const;

	void setColorLeft(const Color& colorLeft);

	void setColorRight(const Color& colorRight);

	void setColorText(const Color& colorText);

private:
	Color m_colorLeft;
	Color m_colorRight;
	Color m_colorText;

	void eventPaint(Event* event);
};

		}
	}
}

#endif	// traktor_ui_custom_GradientStatic_H

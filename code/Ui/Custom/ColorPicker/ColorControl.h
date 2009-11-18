#ifndef traktor_ui_custom_ColorControl_H
#define traktor_ui_custom_ColorControl_H

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
		namespace custom
		{

/*! \brief Color static control.
 * \ingroup UIC
 */
class T_DLLCLASS ColorControl : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, int style);

	void setColor(const Color& color);

	Color getColor() const;

	virtual Size getPreferedSize() const;

private:
	Color m_color;
	Ref< Bitmap > m_preview;

	void eventPaint(Event* event);
};

		}
	}
}

#endif	// traktor_ui_custom_ColorControl_H

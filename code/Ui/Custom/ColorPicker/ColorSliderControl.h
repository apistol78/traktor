#ifndef traktor_ui_custom_ColorSliderControl_H
#define traktor_ui_custom_ColorSliderControl_H

#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

/*! \brief Color slider control.
 * \ingroup UIC
 */
class T_DLLCLASS ColorSliderControl : public Widget
{
	T_RTTI_CLASS;

public:
	struct IGradient : public Object
	{
		virtual Color4ub get(int at) const = 0;
	};

	bool create(Widget* parent, int style, IGradient* gradient);

	virtual Size getPreferedSize() const;

	void updateGradient();

private:
	Ref< IGradient > m_gradient;
	Ref< Bitmap > m_gradientBitmap;

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventMouseMove(MouseMoveEvent* event);

	void eventPaint(PaintEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_ColorSliderControl_H

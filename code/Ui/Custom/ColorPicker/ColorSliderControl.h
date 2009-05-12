#ifndef traktor_ui_custom_ColorSliderControl_H
#define traktor_ui_custom_ColorSliderControl_H

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

/*! \brief Color slider control.
 * \ingroup UIC
 */
class T_DLLCLASS ColorSliderControl : public Widget
{
	T_RTTI_CLASS(ColorSliderControl)

public:
	struct IGradient : public Object
	{
		virtual Color get(int at) const = 0;
	};

	enum Events
	{
		EiColorSelect = EiUser + 1
	};

	bool create(Widget* parent, int style, IGradient* gradient);

	virtual Size getPreferedSize() const;

	void updateGradient();

	void addColorSelectEventHandler(EventHandler* eventHandler);

private:
	Ref< IGradient > m_gradient;
	Ref< Bitmap > m_gradientBitmap;

	void eventButtonDown(Event* event);

	void eventButtonUp(Event* event);

	void eventMouseMove(Event* event);

	void eventPaint(Event* event);
};

		}
	}
}

#endif	// traktor_ui_custom_ColorSliderControl_H

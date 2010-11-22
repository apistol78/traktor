#ifndef traktor_ui_custom_ColorGradientControl_H
#define traktor_ui_custom_ColorGradientControl_H

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

/*! \brief Color gradient control.
 * \ingroup UIC
 */
class T_DLLCLASS ColorGradientControl : public Widget
{
	T_RTTI_CLASS;

public:
	enum Events
	{
		EiColorSelect = EiUser + 1
	};

	bool create(Widget* parent, int style, const Color4ub& color);

	virtual Size getPreferedSize() const;

	void setColor(const Color4ub& color, bool updateCursor);

	Color4ub getColor() const;

	void addColorSelectEventHandler(EventHandler* eventHandler);

private:
	float m_hue;
	Point m_cursor;
	Ref< Bitmap > m_gradientBitmap;

	void updateGradientImage();

	void eventButtonDown(Event* event);

	void eventButtonUp(Event* event);

	void eventMouseMove(Event* event);

	void eventPaint(Event* event);
};

		}
	}
}

#endif	// traktor_ui_custom_ColorGradientControl_H

#ifndef traktor_ui_Slider_H
#define traktor_ui_Slider_H

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
	namespace ui
	{

/*! \brief Slider
 * \ingroup UI
 */
class T_DLLCLASS Slider : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, int style = WsNone);

	void setRange(int minValue, int maxValue);

	void setValue(int value);

	int getValue() const;
};

	}
}

#endif	// traktor_ui_Slider_H

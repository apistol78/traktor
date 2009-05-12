#ifndef traktor_ui_ISlider_H
#define traktor_ui_ISlider_H

#include "Ui/Itf/IWidget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

/*! \brief Slider interface.
 * \ingroup UI
 */
class T_DLLCLASS ISlider : public IWidget
{
public:
	virtual bool create(IWidget* parent, int style) = 0;

	virtual void setRange(int minValue, int maxValue) = 0;

	virtual void setValue(int value) = 0;

	virtual int getValue() const = 0;
};

	}
}

#endif	// traktor_ui_ISlider_H

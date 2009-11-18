#ifndef traktor_ui_custom_CenterLayout_H
#define traktor_ui_custom_CenterLayout_H

#include "Ui/Layout.h"

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

/*! \brief Center layout.
 * \ingroup UIC
 */
class T_DLLCLASS CenterLayout : public Layout
{
	T_RTTI_CLASS;

public:
	virtual bool fit(Widget* widget, const Size& bounds, Size& result);

	virtual void update(Widget* widget);
};

		}
	}
}

#endif	// traktor_ui_custom_CenterLayout_H

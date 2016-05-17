#ifndef traktor_ui_FlowLayout_H
#define traktor_ui_FlowLayout_H

#include <utility>
#include <vector>
#include "Ui/Layout.h"
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

/*! \brief Flow layout.
 * \ingroup UI
 */
class T_DLLCLASS FlowLayout : public Layout
{
	T_RTTI_CLASS;

public:
	FlowLayout();

	FlowLayout(int marginX, int marginY, int padX, int padY);
	
	virtual bool fit(Widget* widget, const Size& bounds, Size& result) T_OVERRIDE;
	
	virtual void update(Widget* widget) T_OVERRIDE;
	
private:
	Size m_margin;
	Size m_pad;
	
	void calculateRects(Widget* widget, std::vector< WidgetRect >& outRects);
};
	
	}
}

#endif	// traktor_ui_FlowLayout_H

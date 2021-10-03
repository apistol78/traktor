#pragma once

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

/*! Flow layout.
 * \ingroup UI
 */
class T_DLLCLASS FlowLayout : public Layout
{
	T_RTTI_CLASS;

public:
	FlowLayout();

	explicit FlowLayout(int32_t marginX, int32_t marginY, int32_t padX, int32_t padY);

	virtual bool fit(Widget* widget, const Size& bounds, Size& result) override;

	virtual void update(Widget* widget) override;

private:
	Size m_margin;
	Size m_pad;

	bool calculateRects(Widget* widget, std::vector< WidgetRect >& outRects) const;
};

	}
}


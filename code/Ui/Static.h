#pragma once

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

/*! \brief Static text.
 * \ingroup UI
 */
class T_DLLCLASS Static : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, const std::wstring& text = L"");

	virtual Size getPreferedSize() const override;

	virtual Size getMaximumSize() const override;

private:
	void eventPaint(PaintEvent* event);
};

	}
}


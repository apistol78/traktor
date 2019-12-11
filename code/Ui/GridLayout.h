#pragma once

#include "Ui/Layout.h"
#include "Ui/Size.h"

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

/*! Grid layout.
 * \ingroup UI
 */
class T_DLLCLASS GridLayout : public Layout
{
	T_RTTI_CLASS;

public:
	GridLayout(int columns, int rows);

	virtual bool fit(Widget* widget, const Size& bounds, Size& result) override;

	virtual void update(Widget* widget) override;

private:
	int m_columns;
	int m_rows;
};

	}
}


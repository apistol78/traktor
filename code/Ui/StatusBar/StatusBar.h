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

/*! Status bar control.
 * \ingroup UI
 */
class T_DLLCLASS StatusBar : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, int style = WsNone);

	void setAlert(bool alert);

	void addColumn(int32_t width);

	void setText(int32_t column, const std::wstring& text);

	virtual Size getPreferedSize() const override;

private:
	struct Column
	{
		int32_t width;
		std::wstring text;
	};

	AlignedVector< Column > m_columns;
	bool m_alert = false;

	void eventSize(SizeEvent* event);

	void eventPaint(PaintEvent* event);
};

	}
}


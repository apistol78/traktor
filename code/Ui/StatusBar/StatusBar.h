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
	StatusBar();

	bool create(Widget* parent, int style = WsNone);

	void setAlert(bool alert);

	virtual void setText(const std::wstring& text) override;

	virtual Size getPreferedSize() const override;

private:
	bool m_alert;

	void eventSize(SizeEvent* event);

	void eventPaint(PaintEvent* event);
};

	}
}


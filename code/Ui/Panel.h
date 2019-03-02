#pragma once

#include "Ui/Container.h"

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

/*! \brief Panel control.
 * \ingroup UI
 */
class T_DLLCLASS Panel : public Container
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, const std::wstring& text, Layout* layout);

	virtual void destroy() override;

	virtual Size getMinimumSize() const override;

	virtual Size getPreferedSize() const override;

	virtual Rect getInnerRect() const override;

private:
	Ref< ui::EventSubject::IEventHandler > m_focusEventHandler;

	void eventPaint(PaintEvent* event);

	void eventFocus(FocusEvent* event);
};

	}
}


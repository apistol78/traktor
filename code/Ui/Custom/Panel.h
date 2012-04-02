#ifndef traktor_ui_custom_Panel_H
#define traktor_ui_custom_Panel_H

#include "Ui/Container.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Event;

		namespace custom
		{

/*! \brief Panel control.
 * \ingroup UIC
 */
class T_DLLCLASS Panel : public Container
{
	T_RTTI_CLASS;

public:
	Panel();

	bool create(Widget* parent, const std::wstring& text, Layout* layout);

	virtual void destroy();

	virtual Size getMinimumSize() const;

	virtual Size getPreferedSize() const;

	virtual Rect getInnerRect() const;

private:
	Ref< EventHandler > m_focusHandler;

	void eventPaint(Event* event);

	void eventFocus(Event* event);
};

		}
	}
}

#endif	// traktor_ui_custom_Panel_H

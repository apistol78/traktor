#ifndef traktor_ui_MenuBar_H
#define traktor_ui_MenuBar_H

#include "Ui/EventSubject.h"

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

class Form;
class MenuItem;
class IMenuBar;

/*! \brief Menu bar.
 * \ingroup UI
 */
class T_DLLCLASS MenuBar : public EventSubject
{
	T_RTTI_CLASS;

public:
	MenuBar();

	virtual ~MenuBar();

	bool create(Form* form);

	void destroy();

	void add(MenuItem* item);
	
private:
	IMenuBar* m_menuBar;
	Form* m_form;
};

	}
}

#endif	// traktor_ui_MenuBar_H

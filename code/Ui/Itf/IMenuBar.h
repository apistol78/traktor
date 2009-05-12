#ifndef traktor_ui_IMenuBar_H
#define traktor_ui_IMenuBar_H

#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class IForm;
class MenuItem;

/*! \brief MenuBar interface.
 * \ingroup UI
 */
class T_DLLCLASS IMenuBar
{
public:
	virtual bool create(IForm* form) = 0;

	virtual void destroy() = 0;

	virtual void add(MenuItem* item) = 0;
};

	}
}

#endif	// traktor_ui_IMenuBar_H

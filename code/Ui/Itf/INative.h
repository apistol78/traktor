#ifndef traktor_ui_INative_H
#define traktor_ui_INative_H

#include "Ui/Itf/IWidget.h"

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

/*! \brief Native interface.
 * \ingroup UI
 */
class T_DLLCLASS INative : public IWidget
{
public:
	virtual bool create(void* nativeHandle) = 0;
};

	}
}

#endif	// traktor_ui_INative_H

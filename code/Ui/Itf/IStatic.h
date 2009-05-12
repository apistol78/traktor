#ifndef traktor_ui_IStatic_H
#define traktor_ui_IStatic_H

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

/*! \brief Static interface.
 * \ingroup UI
 */
class T_DLLCLASS IStatic : public IWidget
{
public:
	virtual bool create(IWidget* parent, const std::wstring& text) = 0;
};

	}
}

#endif	// traktor_ui_IStatic_H

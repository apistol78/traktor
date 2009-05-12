#ifndef traktor_ui_IButton_H
#define traktor_ui_IButton_H

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

class Command;

/*! \brief Button interface.
 * \ingroup UI
 */
class T_DLLCLASS IButton : public IWidget
{
public:
	virtual bool create(IWidget* parent, const std::wstring& text, int style) = 0;

	virtual void setState(bool state) = 0;

	virtual bool getState() const = 0;
};

	}
}

#endif	// traktor_ui_IButton_H

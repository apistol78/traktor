#ifndef traktor_ui_IForm_H
#define traktor_ui_IForm_H

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

class EventSubject;
class IBitmap;

/*! \brief Form interface.
 * \ingroup UI
 */
class T_DLLCLASS IForm : public IWidget
{
public:
	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style) = 0;

	virtual void setIcon(IBitmap* icon) = 0;

	virtual void maximize() = 0;

	virtual void minimize() = 0;

	virtual void restore() = 0;

	virtual bool isMaximized() const = 0;

	virtual bool isMinimized() const = 0;
};

	}
}

#endif	// traktor_ui_IForm_H

#ifndef traktor_ui_IEdit_H
#define traktor_ui_IEdit_H

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

/*! \brief Edit interface.
 * \ingroup UI
 */
class T_DLLCLASS IEdit : public IWidget
{
public:
	virtual bool create(IWidget* parent, const std::wstring& text, int style) = 0;

	virtual void setSelection(int from, int to) = 0;

	virtual void getSelection(int& outFrom, int& outTo) const = 0;

	virtual void selectAll() = 0;
};

	}
}

#endif	// traktor_ui_IEdit_H

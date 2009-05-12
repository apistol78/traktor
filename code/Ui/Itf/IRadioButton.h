#ifndef traktor_ui_IRadioButton_H
#define traktor_ui_IRadioButton_H

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

/*! \brief RadioButton interface.
 * \ingroup UI
 */
class T_DLLCLASS IRadioButton : public IWidget
{
public:
	virtual bool create(IWidget* parent, const std::wstring& text, bool checked) = 0;

	virtual void setChecked(bool checked) = 0;

	virtual bool isChecked() const = 0;
};

	}
}

#endif	// traktor_ui_IRadioButton_H

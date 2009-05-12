#ifndef traktor_ui_ICheckBox_H
#define traktor_ui_ICheckBox_H

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

/*! \brief CheckBox interface.
 * \ingroup UI
 */
class T_DLLCLASS ICheckBox : public IWidget
{
public:
	virtual bool create(IWidget* parent, const std::wstring& text, bool checked) = 0;

	virtual void setChecked(bool checked) = 0;

	virtual bool isChecked() const = 0;
};

	}
}

#endif	// traktor_ui_ICheckBox_H

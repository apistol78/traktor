#ifndef traktor_ui_IComboBox_H
#define traktor_ui_IComboBox_H

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

/*! \brief ComboBox interface.
 * \ingroup UI
 */
class T_DLLCLASS IComboBox : public IWidget
{
public:
	virtual bool create(IWidget* parent, const std::wstring& text, int style) = 0;

	virtual int add(const std::wstring& item) = 0;

	virtual bool remove(int index) = 0;

	virtual void removeAll() = 0;

	virtual int count() const = 0;

	virtual std::wstring get(int index) const = 0;
	
	virtual void select(int index) = 0;

	virtual int getSelected() const = 0;
};

	}
}

#endif	// traktor_ui_IComboBox_H

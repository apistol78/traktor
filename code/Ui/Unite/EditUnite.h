#ifndef traktor_ui_EditUnite_H
#define traktor_ui_EditUnite_H

#include "Ui/Itf/IEdit.h"
#include "Ui/Unite/WidgetUniteImpl.h"

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIW32
 */
class EditUnite : public WidgetUniteImpl< IEdit >
{
public:
	EditUnite(IWidgetFactory* nativeWidgetFactory, EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int style);

	virtual void setSelection(int from, int to);

	virtual void getSelection(int& outFrom, int& outTo) const;

	virtual void selectAll();

	virtual Size getPreferedSize() const;
};

	}
}

#endif	// traktor_ui_EditUnite_H

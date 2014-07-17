#ifndef traktor_ui_Edit_H
#define traktor_ui_Edit_H

#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class EditValidator;

/*! \brief Text edit.
 * \ingroup UI
 */
class T_DLLCLASS Edit : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, const std::wstring& text = L"", int style = WsClientBorder, EditValidator* validator = 0);
	
	bool setValidator(EditValidator* validator);

	Ref< EditValidator > getValidator() const;

	void setSelection(int from, int to);

	void selectAll();

private:
	Ref< EditValidator > m_validator;

	void eventKey(KeyEvent* event);
};

	}
}

#endif	// traktor_ui_Edit_H

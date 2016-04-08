#ifndef traktor_ui_CheckBox_H
#define traktor_ui_CheckBox_H

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

/*! \brief CheckBox
 * \ingroup UI
 */
class T_DLLCLASS CheckBox : public Widget
{
	T_RTTI_CLASS;

public:
	CheckBox();

	bool create(Widget* parent, const std::wstring& text = L"", bool checked = false);
	
	void setChecked(bool checked);
	
	bool isChecked() const;

	virtual Size getPreferedSize() const;

private:
	bool m_checked;

	void eventPaint(PaintEvent* event);

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);
};

	}
}

#endif	// traktor_ui_CheckBox_H

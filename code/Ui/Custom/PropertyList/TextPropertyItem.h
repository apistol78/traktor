#ifndef traktor_ui_custom_TextPropertyItem_H
#define traktor_ui_custom_TextPropertyItem_H

#include "Ui/Point.h"
#include "Ui/Custom/PropertyList/PropertyItem.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Edit;

		namespace custom
		{

class MiniButton;

/*! \brief Text property item.
 * \ingroup UIC
 */
class T_DLLCLASS TextPropertyItem : public PropertyItem
{
	T_RTTI_CLASS;

public:
	TextPropertyItem(const std::wstring& text, const std::wstring& value, bool multiLine);

	void setValue(const std::wstring& value);

	const std::wstring& getValue() const;

protected:
	virtual void createInPlaceControls(Widget* parent);

	virtual void destroyInPlaceControls();

	virtual void resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects);

	virtual void mouseButtonDown(MouseButtonDownEvent* event);

	virtual void paintValue(Canvas& canvas, const Rect& rc);

	virtual bool copy();

	virtual bool paste();

private:
	Ref< Edit > m_editor;
	Ref< MiniButton > m_buttonEdit;
	std::wstring m_value;
	bool m_multiLine;

	void eventEditFocus(FocusEvent* event);

	void eventClick(ButtonClickEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_TextPropertyItem_H

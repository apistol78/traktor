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
	virtual void createInPlaceControls(Widget* parent) T_OVERRIDE;

	virtual void destroyInPlaceControls() T_OVERRIDE;

	virtual void resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects) T_OVERRIDE;

	virtual void mouseButtonDown(MouseButtonDownEvent* event) T_OVERRIDE;

	virtual void paintValue(Canvas& canvas, const Rect& rc) T_OVERRIDE;

	virtual bool copy() T_OVERRIDE;

	virtual bool paste() T_OVERRIDE;

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

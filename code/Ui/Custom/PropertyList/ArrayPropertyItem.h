#ifndef traktor_ui_custom_ArrayPropertyItem_H
#define traktor_ui_custom_ArrayPropertyItem_H

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
		namespace custom
		{

class MiniButton;

/*! \brief Array property item.
 * \ingroup UIC
 */
class T_DLLCLASS ArrayPropertyItem : public PropertyItem
{
	T_RTTI_CLASS;
	
public:
	ArrayPropertyItem(const std::wstring& text, const TypeInfo* elementType, bool readOnly);

	void setElementType(const TypeInfo* objectType);

	const TypeInfo* getElementType() const;

protected:
	virtual bool needRemoveChildButton() const;

	virtual void createInPlaceControls(Widget* parent);

	virtual void destroyInPlaceControls();

	virtual void resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects);

	virtual void paintValue(Canvas& canvas, const Rect& rc);

private:
	const TypeInfo* m_elementType;
	bool m_readOnly;
	Ref< MiniButton > m_buttonEdit;

	void eventClick(ButtonClickEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_ArrayPropertyItem_H

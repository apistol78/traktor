#ifndef traktor_ui_custom_BrowsePropertyItem_H
#define traktor_ui_custom_BrowsePropertyItem_H

#include "Ui/Custom/PropertyList/PropertyItem.h"
#include "Core/Guid.h"

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

/*! \brief Browse property item.
 * \ingroup UIC
 */
class T_DLLCLASS BrowsePropertyItem : public PropertyItem
{
	T_RTTI_CLASS;

public:
	BrowsePropertyItem(const std::wstring& text, const TypeInfo* filterType, const Guid& value);

	void setFilterType(const TypeInfo* filterType);

	const TypeInfo* getFilterType() const;

	void setValue(const Guid& text);

	const Guid& getValue() const;

protected:
	virtual void createInPlaceControls(Widget* parent);

	virtual void destroyInPlaceControls();

	virtual void resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects);

	virtual void paintValue(Canvas& canvas, const Rect& rc);

	virtual bool copy();

	virtual bool paste();

private:
	Guid m_value;
	const TypeInfo* m_filterType;
	Ref< MiniButton > m_buttonEdit;
	Ref< MiniButton > m_buttonBrowse;

	void eventEditClick(ButtonClickEvent* event);

	void eventBrowseClick(ButtonClickEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_BrowsePropertyItem_H

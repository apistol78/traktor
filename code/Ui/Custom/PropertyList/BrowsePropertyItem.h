#ifndef traktor_ui_custom_BrowsePropertyItem_H
#define traktor_ui_custom_BrowsePropertyItem_H

#include "Ui/Custom/PropertyList/PropertyItem.h"
#include "Core/Guid.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Event;

		namespace custom
		{

class MiniButton;

/*! \brief Browse property item.
 * \ingroup UIC
 */
class T_DLLCLASS BrowsePropertyItem : public PropertyItem
{
	T_RTTI_CLASS(BrowsePropertyItem)

public:
	BrowsePropertyItem(const std::wstring& text, const Type* filterType, const Guid& value);

	void setFilterType(const Type* filterType);

	const Type* getFilterType() const;

	void setValue(const Guid& text);

	const Guid& getValue() const;

protected:
	virtual void createInPlaceControls(Widget* parent, bool visible);

	virtual void destroyInPlaceControls();

	virtual void resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects);

	virtual void showInPlaceControls(bool show);

	virtual void paintValue(Canvas& canvas, const Rect& rc);

private:
	Guid m_value;
	const Type* m_filterType;
	Ref< MiniButton > m_buttonEdit;

	void eventClick(Event* event);
};

		}
	}
}

#endif	// traktor_ui_custom_BrowsePropertyItem_H

#ifndef traktor_ui_custom_ObjectPropertyItem_H
#define traktor_ui_custom_ObjectPropertyItem_H

#include "Ui/Custom/PropertyList/PropertyItem.h"

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

/*! \brief Object property item.
 * \ingroup UIC
 */
class T_DLLCLASS ObjectPropertyItem : public PropertyItem
{
	T_RTTI_CLASS(ObjectPropertyItem)

public:
	ObjectPropertyItem(const std::wstring& text, const Type* objectType, Object* object);

	void setObjectType(const Type* objectType);

	const Type* getObjectType() const;

	void setObject(Object* object);

	Object* getObject() const;

protected:
	virtual void createInPlaceControls(Widget* parent);

	virtual void destroyInPlaceControls();

	virtual void resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects);

	virtual void paintValue(Canvas& canvas, const Rect& rc);

private:
	Ref< MiniButton > m_buttonEdit;
	const Type* m_objectType;
	Ref< Object > m_object;

	void eventClick(Event* event);
};

		}
	}
}

#endif	// traktor_ui_custom_ObjectPropertyItem_H

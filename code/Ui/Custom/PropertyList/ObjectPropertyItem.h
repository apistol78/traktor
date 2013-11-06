#ifndef traktor_ui_custom_ObjectPropertyItem_H
#define traktor_ui_custom_ObjectPropertyItem_H

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

class Event;

		namespace custom
		{

class MiniButton;

/*! \brief Object property item.
 * \ingroup UIC
 */
class T_DLLCLASS ObjectPropertyItem : public PropertyItem
{
	T_RTTI_CLASS;

public:
	ObjectPropertyItem(const std::wstring& text, const TypeInfo* objectType, Object* object);

	void setObjectType(const TypeInfo* objectType);

	const TypeInfo* getObjectType() const;

	void setObject(Object* object);

	Ref< Object > getObject() const;

protected:
	virtual void createInPlaceControls(Widget* parent);

	virtual void destroyInPlaceControls();

	virtual void resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects);

	virtual void paintValue(Canvas& canvas, const Rect& rc);

	virtual bool copy();

	virtual bool paste();

private:
	Ref< MiniButton > m_buttonEdit;
	const TypeInfo* m_objectType;
	Ref< Object > m_object;

	void eventClick(Event* event);
};

		}
	}
}

#endif	// traktor_ui_custom_ObjectPropertyItem_H

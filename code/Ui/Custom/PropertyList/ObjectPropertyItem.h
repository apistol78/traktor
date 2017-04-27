/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
	virtual void createInPlaceControls(Widget* parent) T_OVERRIDE;

	virtual void destroyInPlaceControls() T_OVERRIDE;

	virtual void resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects) T_OVERRIDE;

	virtual void paintValue(Canvas& canvas, const Rect& rc) T_OVERRIDE;

	virtual bool copy() T_OVERRIDE;

	virtual bool paste() T_OVERRIDE;

private:
	Ref< MiniButton > m_buttonEdit;
	const TypeInfo* m_objectType;
	Ref< Object > m_object;

	void eventClick(ButtonClickEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_ObjectPropertyItem_H

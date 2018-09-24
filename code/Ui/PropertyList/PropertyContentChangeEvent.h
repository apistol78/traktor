/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_PropertyContentChangeEvent_H
#define traktor_ui_PropertyContentChangeEvent_H

#include "Ui/Events/ContentChangeEvent.h"

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

class PropertyItem;

/*! \brief
 * \ingroup UI
 */
class T_DLLCLASS PropertyContentChangeEvent : public ContentChangeEvent
{
	T_RTTI_CLASS;
	
public:
	PropertyContentChangeEvent(EventSubject* sender, PropertyItem* item);
	
	PropertyItem* getItem() const;

private:
	Ref< PropertyItem > m_item;
};

	}
}

#endif	// traktor_ui_PropertyContentChangeEvent_H

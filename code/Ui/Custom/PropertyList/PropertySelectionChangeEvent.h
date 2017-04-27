/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_PropertySelectionChangeEvent_H
#define traktor_ui_custom_PropertySelectionChangeEvent_H

#include "Ui/Events/SelectionChangeEvent.h"

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

class PropertyItem;

/*! \brief
 * \ingroup UIC
 */
class T_DLLCLASS PropertySelectionChangeEvent : public SelectionChangeEvent
{
	T_RTTI_CLASS;
	
public:
	PropertySelectionChangeEvent(EventSubject* sender, PropertyItem* item, int32_t index);
	
	PropertyItem* getItem() const;

	int32_t getIndex() const;

private:
	Ref< PropertyItem > m_item;
	int32_t m_index;
};

		}
	}
}

#endif	// traktor_ui_custom_PropertySelectionChangeEvent_H

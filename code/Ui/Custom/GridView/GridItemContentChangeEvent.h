/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_GridItemContentChangeEvent_H
#define traktor_ui_custom_GridItemContentChangeEvent_H

#include "Ui/Events/ContentChangeEvent.h"

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

class GridItem;
	
/*! \brief
 * \ingroup UIC
 */
class T_DLLCLASS GridItemContentChangeEvent : public ContentChangeEvent
{
	T_RTTI_CLASS;
	
public:
	GridItemContentChangeEvent(EventSubject* sender, GridItem* item);

	GridItem* getItem() const;

private:
	Ref< GridItem > m_item;
};
	
		}
	}
}

#endif	// traktor_ui_custom_GridItemContentChangeEvent_H

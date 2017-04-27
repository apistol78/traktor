/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_KeyMoveEvent_H
#define traktor_ui_custom_KeyMoveEvent_H

#include "Ui/Event.h"

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

class Key;

/*! \brief
 * \ingroup UIC
 */
class T_DLLCLASS KeyMoveEvent : public Event
{
	T_RTTI_CLASS;

public:
	KeyMoveEvent(EventSubject* sender, Key* key, int32_t offset);

	Key* getKey() const;

	int32_t getOffset() const;

private:
	Ref< Key > m_key;
	int32_t m_offset;
};

		}
	}
}

#endif	// traktor_ui_custom_KeyMoveEvent_H

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_EditListEditEvent_H
#define traktor_ui_custom_EditListEditEvent_H

#include <string>
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

class Edit;

		namespace custom
		{
	
/*! \brief Edit event.
 * \ingroup UIC
 */
class T_DLLCLASS EditListEditEvent : public ContentChangeEvent
{
	T_RTTI_CLASS;
	
public:
	EditListEditEvent(EventSubject* sender, Edit* edit, int32_t index, const std::wstring& text);
	
	Edit* getEdit() const;

	int32_t getIndex() const;

	const std::wstring& getText() const;

private:
	Ref< Edit > m_edit;
	int32_t m_index;
	std::wstring m_text;
};

		}
	}
}

#endif	// traktor_ui_custom_EditListEditEvent_H

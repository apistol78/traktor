/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_SearchEvent_H
#define traktor_ui_custom_SearchEvent_H

#include <string>
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

class T_DLLCLASS SearchEvent : public ui::Event
{
	T_RTTI_CLASS;

public:
	SearchEvent(
		ui::EventSubject* sender,
		bool preview
	);

	bool preview() const { return m_preview; }

private:
	std::wstring m_search;
	bool m_preview;
};

		}
	}
}

#endif	// traktor_ui_custom_SearchEvent_H

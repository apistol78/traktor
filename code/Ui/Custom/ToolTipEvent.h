/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_ToolTipEvent_H
#define traktor_ui_custom_ToolTipEvent_H

#include "Ui/Event.h"
#include "Ui/Point.h"

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

/*! \brief Tool tip event.
 * \ingroup UIC
 */
class T_DLLCLASS ToolTipEvent : public Event
{
	T_RTTI_CLASS;

public:
	ToolTipEvent(EventSubject* sender, const Point& position);

	void setPosition(const Point& position);

	const Point& getPosition() const;

	void setText(const std::wstring& text);

	const std::wstring& getText() const;

private:
	Point m_position;
	std::wstring m_text;
};

		}
	}
}

#endif	// traktor_ui_custom_ToolTipEvent_H

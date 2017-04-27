/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_ToolTip_H
#define traktor_ui_custom_ToolTip_H

#include "Ui/ToolForm.h"

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

/*! \brief Tool tip control.
 * \ingroup UIC
 */
class T_DLLCLASS ToolTip : public ToolForm
{
	T_RTTI_CLASS;

public:
	ToolTip();

	bool create(Widget* parent);

	void show(const Point& at, const std::wstring& text);

private:
	bool m_tracking;
	uint32_t m_counter;

	void eventTimer(TimerEvent* event);

	void eventPaint(PaintEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_ToolTip_H

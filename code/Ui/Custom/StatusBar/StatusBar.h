/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_StatusBar_H
#define traktor_ui_custom_StatusBar_H

#include "Ui/Widget.h"

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

/*! \brief Status bar control.
 * \ingroup UIC
 */
class T_DLLCLASS StatusBar : public Widget
{
	T_RTTI_CLASS;

public:
	StatusBar();

	bool create(Widget* parent, int style = WsNone);

	void setAlert(bool alert);

	virtual void setText(const std::wstring& text) T_OVERRIDE;

	virtual Size getPreferedSize() const T_OVERRIDE;

private:
	bool m_alert;

	void eventSize(SizeEvent* event);

	void eventPaint(PaintEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_StatusBar_H

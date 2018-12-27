/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_GradientStatic_H
#define traktor_ui_GradientStatic_H

#include "Ui/Widget.h"

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

/*! \brief Gradient static control.
 * \ingroup UI
 */
class T_DLLCLASS GradientStatic : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, const std::wstring& text, int style = WsNone);

	virtual Size getPreferedSize() const override;

private:
	void eventPaint(PaintEvent* event);
};

	}
}

#endif	// traktor_ui_GradientStatic_H

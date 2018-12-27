/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_Panel_H
#define traktor_ui_Panel_H

#include "Ui/Container.h"

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

/*! \brief Panel control.
 * \ingroup UI
 */
class T_DLLCLASS Panel : public Container
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, const std::wstring& text, Layout* layout);

	virtual void destroy() override;

	virtual Size getMinimumSize() const override;

	virtual Size getPreferedSize() const override;

	virtual Rect getInnerRect() const override;

private:
	Ref< ui::EventSubject::IEventHandler > m_focusEventHandler;

	void eventPaint(PaintEvent* event);

	void eventFocus(FocusEvent* event);
};

	}
}

#endif	// traktor_ui_Panel_H

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_PanelWin32_H
#define traktor_ui_PanelWin32_H

#include "Ui/Itf/IPanel.h"
#include "Ui/Win32/WidgetWin32Impl.h"

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIW32
 */
class PanelWin32 : public WidgetWin32Impl< IPanel >
{
public:
	PanelWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text);

	virtual void update(const Rect* rc, bool immediate);

	virtual Rect getInnerRect() const;

	virtual Size getMinimumSize() const;
	
	virtual Size getPreferedSize() const;
	
	virtual Size getMaximumSize() const;
};

	}
}

#endif	// traktor_ui_PanelWin32_H

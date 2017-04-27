/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_StaticWin32_H
#define traktor_ui_StaticWin32_H

#include "Ui/Itf/IStatic.h"
#include "Ui/Win32/WidgetWin32Impl.h"

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIW32
 */
class StaticWin32 : public WidgetWin32Impl< IStatic >
{
public:
	StaticWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text);

	virtual Size getPreferedSize() const;
};

	}
}

#endif	// traktor_ui_StaticWin32_H

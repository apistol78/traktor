/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_ContainerWin32_H
#define traktor_ui_ContainerWin32_H

#include "Ui/Itf/IContainer.h"
#include "Ui/Win32/WidgetWin32Impl.h"

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIW32
 */
class ContainerWin32 : public WidgetWin32Impl< IContainer >
{
public:
	ContainerWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, int style);
};

	}
}

#endif	// traktor_ui_ContainerWin32_H

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_NativeWin32_H
#define traktor_ui_NativeWin32_H

#include "Ui/Itf/INative.h"
#include "Ui/Win32/WidgetWin32Impl.h"

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIW32
 */
class NativeWin32 : public WidgetWin32Impl< INative >
{
public:
	NativeWin32(EventSubject* owner);

	virtual bool create(void* nativeHandle);
};

	}
}

#endif	// traktor_ui_NativeWin32_H

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_UtilitiesX11_H
#define traktor_ui_UtilitiesX11_H

#include <X11/Xlib.h>
#include "Core/Config.h"
#include "Ui/Enums.h"

namespace traktor
{
	namespace ui
	{

/*! \brief Translate X11 virtual key to our virtual key.
 * \ingroup UIW32
 *
 * \param keySym X11 virtual key symbols.
 * \param nkeySyms Number of key symbols
 * \return Virtual key.
 */
VirtualKey translateToVirtualKey(const KeySym* keySym, int nkeySyms);

	}
}

#endif	// traktor_ui_UtilitiesX11_H

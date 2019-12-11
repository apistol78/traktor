#pragma once

#include <X11/Xlib.h>
#include "Core/Config.h"
#include "Ui/Enums.h"

namespace traktor
{
	namespace ui
	{

/*! Translate X11 virtual key to our virtual key.
 * \ingroup UIW32
 *
 * \param keySym X11 virtual key symbols.
 * \param nkeySyms Number of key symbols
 * \return Virtual key.
 */
VirtualKey translateToVirtualKey(const KeySym* keySym, int nkeySyms);

	}
}


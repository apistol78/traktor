#pragma once

#include "Core/Config.h"
#include "Ui/Enums.h"

namespace traktor
{
	namespace ui
	{

/*! Translate Win32 virtual key to our virtual key.
 * \ingroup UIW32
 *
 * \param keyCode Win32 virtual key.
 * \return Virtual key.
 */
VirtualKey translateToVirtualKey(uint32_t keyCode);

/*! Translate virtual key to Win32 virtual key.
 * \ingroup UIW32
 *
 * \param vk Virtual key.
 * \return Key code.
 */
uint32_t translateToKeyCode(VirtualKey vk);

	}
}


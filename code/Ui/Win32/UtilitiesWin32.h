#ifndef traktor_ui_UtilitiesWin32_H
#define traktor_ui_UtilitiesWin32_H

#include "Core/Config.h"
#include "Ui/Enums.h"

namespace traktor
{
	namespace ui
	{

/*! \brief Translate Win32 virtual key to our virtual key.
 * \ingroup UIW32
 *
 * \param keyCode Win32 virtual key.
 * \return Virtual key.
 */
VirtualKey translateToVirtualKey(uint32_t keyCode);

/*! \brief Translate virtual key to Win32 virtual key.
 * \ingroup UIW32
 *
 * \param vk Virtual key.
 * \return Key code.
 */
uint32_t translateToKeyCode(VirtualKey vk);

	}
}

#endif	// traktor_ui_UtilitiesWin32_H

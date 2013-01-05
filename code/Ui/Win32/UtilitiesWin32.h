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
VirtualKey translateKeyCode(uint32_t keyCode);
	
	}
}

#endif	// traktor_ui_UtilitiesWin32_H

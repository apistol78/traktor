#ifndef traktor_ui_UtilitiesWin32_H
#define traktor_ui_UtilitiesWin32_H

#include "Ui/Enums.h"

namespace traktor
{
	namespace ui
	{

/*! \brief Translate Win32 virtual key to our virtual key.
 *
 * \param keyCode Win32 virtual key.
 * \return Virtual key.
 */
VirtualKey translateKeyCode(uint32_t keyCode);
	
	}
}

#endif	// traktor_ui_UtilitiesWin32_H

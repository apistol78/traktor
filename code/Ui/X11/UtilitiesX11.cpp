/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/keysymdef.h>
#include "Ui/X11/UtilitiesX11.h"

namespace traktor::ui
{
	namespace
	{

const struct { uint32_t keySym; VirtualKey vkey; } c_translateTable[] =
{
	{ XK_Tab, VkTab },
	{ XK_Clear, VkClear },
	{ XK_Return, VkReturn },
	{ XK_Shift_L, VkShift },
	{ XK_Shift_R, VkShift },
	{ XK_Control_L, VkControl },
	{ XK_Control_R, VkControl },
	{ XK_Meta_L, VkMenu },
	{ XK_Pause, VkPause },
	//{ VK_CAPITAL, VkCapital },
	{ XK_Escape, VkEscape },
	{ XK_space, VkSpace },
	{ XK_End, VkEnd },
	{ XK_Home, VkHome },
	{ XK_Page_Up, VkPageUp },
	{ XK_Page_Down, VkPageDown },
	{ XK_Left, VkLeft },
	{ XK_Up, VkUp },
	{ XK_Right, VkRight },
	{ XK_Down, VkDown },
	{ XK_Select, VkSelect },
	{ XK_Print, VkPrint },
	{ XK_Execute, VkExecute },
	//{ VK_SNAPSHOT, VkSnapshot },
	{ XK_Insert, VkInsert },
	{ XK_Delete, VkDelete },
	{ XK_BackSpace, VkBackSpace },
	{ XK_Help, VkHelp },
	{ XK_KP_0, VkNumPad0 },
	{ XK_KP_1, VkNumPad1 },
	{ XK_KP_2, VkNumPad2 },
	{ XK_KP_3, VkNumPad3 },
	{ XK_KP_4, VkNumPad4 },
	{ XK_KP_5, VkNumPad5 },
	{ XK_KP_6, VkNumPad6 },
	{ XK_KP_7, VkNumPad7 },
	{ XK_KP_8, VkNumPad8 },
	{ XK_KP_9, VkNumPad9 },
	{ XK_multiply, VkMultiply },
	{ XK_KP_Add, VkAdd },
	//{ VK_SEPARATOR, VkSeparator },
	{ XK_KP_Subtract, VkSubtract },
	{ XK_KP_Decimal, VkDecimal },
	//{ VK_DIVIDE, VkDivide },
	{ XK_F1, VkF1 },
	{ XK_F2, VkF2 },
	{ XK_F3, VkF3 },
	{ XK_F4, VkF4 },
	{ XK_F5, VkF5 },
	{ XK_F6, VkF6 },
	{ XK_F7, VkF7 },
	{ XK_F8, VkF8 },
	{ XK_F9, VkF9 },
	{ XK_F10, VkF10 },
	{ XK_F11, VkF11 },
	{ XK_F12, VkF12 },
	//{ VK_NUMLOCK, VkNumLock },
	//{ VK_SCROLL, VkScroll },
	{ XK_comma, VkComma },
	{ XK_period, VkPeriod },
	{ XK_Q, VkQ },
	{ XK_W, VkW },
	{ XK_E, VkE },
	{ XK_R, VkR },
	{ XK_T, VkT },
	{ XK_Y, VkY },
	{ XK_U, VkU },
	{ XK_I, VkI },
	{ XK_O, VkO },
	{ XK_P, VkP },
	{ XK_A, VkA },
	{ XK_S, VkS },
	{ XK_D, VkD },
	{ XK_F, VkF },
	{ XK_G, VkG },
	{ XK_H, VkH },
	{ XK_J, VkJ },
	{ XK_K, VkK },
	{ XK_L, VkL },
	{ XK_Z, VkZ },
	{ XK_X, VkX },
	{ XK_C, VkC },
	{ XK_V, VkV },
	{ XK_B, VkB },
	{ XK_N, VkN },
	{ XK_M, VkM },
	{ XK_1, Vk1 },
	{ XK_2, Vk2 },
	{ XK_3, Vk3 },
	{ XK_4, Vk4 },
	{ XK_5, Vk5 },
	{ XK_6, Vk6 },
	{ XK_7, Vk7 },
	{ XK_8, Vk8 },
	{ XK_9, Vk9 },
	{ XK_0, Vk0 },
};

	}

VirtualKey translateToVirtualKey(const KeySym* keySym, int nkeySyms)
{
	for (int32_t i = 0; i < sizeof_array(c_translateTable); ++i)
	{
		for (int32_t j = 0; j < nkeySyms; ++j)
		{
			if (c_translateTable[i].keySym == keySym[j])
				return c_translateTable[i].vkey;
		}
	}
	return VkNull;
}

}

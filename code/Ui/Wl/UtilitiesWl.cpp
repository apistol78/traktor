/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <xkbcommon/xkbcommon-keysyms.h>
#include "Ui/Wl/UtilitiesWl.h"

namespace traktor::ui
{
	namespace
	{

const struct { xkb_keysym_t keySym; VirtualKey vkey; } c_translateTable[] =
{
	{ XKB_KEY_Tab, VkTab },
	{ XKB_KEY_Clear, VkClear },
	{ XKB_KEY_Return, VkReturn },
	{ XKB_KEY_Shift_L, VkShift },
	{ XKB_KEY_Shift_R, VkShift },
	{ XKB_KEY_Control_L, VkControl },
	{ XKB_KEY_Control_R, VkControl },
	{ XKB_KEY_Meta_L, VkMenu },
	{ XKB_KEY_Pause, VkPause },
	{ XKB_KEY_Escape, VkEscape },
	{ XKB_KEY_space, VkSpace },
	{ XKB_KEY_End, VkEnd },
	{ XKB_KEY_Home, VkHome },
	{ XKB_KEY_Page_Up, VkPageUp },
	{ XKB_KEY_Page_Down, VkPageDown },
	{ XKB_KEY_Left, VkLeft },
	{ XKB_KEY_Up, VkUp },
	{ XKB_KEY_Right, VkRight },
	{ XKB_KEY_Down, VkDown },
	{ XKB_KEY_Select, VkSelect },
	{ XKB_KEY_Print, VkPrint },
	{ XKB_KEY_Execute, VkExecute },
	{ XKB_KEY_Insert, VkInsert },
	{ XKB_KEY_Delete, VkDelete },
	{ XKB_KEY_BackSpace, VkBackSpace },
	{ XKB_KEY_Help, VkHelp },
	{ XKB_KEY_KP_0, VkNumPad0 },
	{ XKB_KEY_KP_1, VkNumPad1 },
	{ XKB_KEY_KP_2, VkNumPad2 },
	{ XKB_KEY_KP_3, VkNumPad3 },
	{ XKB_KEY_KP_4, VkNumPad4 },
	{ XKB_KEY_KP_5, VkNumPad5 },
	{ XKB_KEY_KP_6, VkNumPad6 },
	{ XKB_KEY_KP_7, VkNumPad7 },
	{ XKB_KEY_KP_8, VkNumPad8 },
	{ XKB_KEY_KP_9, VkNumPad9 },
	{ XKB_KEY_multiply, VkMultiply },
	{ XKB_KEY_KP_Add, VkAdd },
	{ XKB_KEY_KP_Subtract, VkSubtract },
	{ XKB_KEY_KP_Decimal, VkDecimal },
	{ XKB_KEY_F1, VkF1 },
	{ XKB_KEY_F2, VkF2 },
	{ XKB_KEY_F3, VkF3 },
	{ XKB_KEY_F4, VkF4 },
	{ XKB_KEY_F5, VkF5 },
	{ XKB_KEY_F6, VkF6 },
	{ XKB_KEY_F7, VkF7 },
	{ XKB_KEY_F8, VkF8 },
	{ XKB_KEY_F9, VkF9 },
	{ XKB_KEY_F10, VkF10 },
	{ XKB_KEY_F11, VkF11 },
	{ XKB_KEY_F12, VkF12 },
	{ XKB_KEY_comma, VkComma },
	{ XKB_KEY_period, VkPeriod },
	{ XKB_KEY_Q, VkQ },
	{ XKB_KEY_W, VkW },
	{ XKB_KEY_E, VkE },
	{ XKB_KEY_R, VkR },
	{ XKB_KEY_T, VkT },
	{ XKB_KEY_Y, VkY },
	{ XKB_KEY_U, VkU },
	{ XKB_KEY_I, VkI },
	{ XKB_KEY_O, VkO },
	{ XKB_KEY_P, VkP },
	{ XKB_KEY_A, VkA },
	{ XKB_KEY_S, VkS },
	{ XKB_KEY_D, VkD },
	{ XKB_KEY_F, VkF },
	{ XKB_KEY_G, VkG },
	{ XKB_KEY_H, VkH },
	{ XKB_KEY_J, VkJ },
	{ XKB_KEY_K, VkK },
	{ XKB_KEY_L, VkL },
	{ XKB_KEY_Z, VkZ },
	{ XKB_KEY_X, VkX },
	{ XKB_KEY_C, VkC },
	{ XKB_KEY_V, VkV },
	{ XKB_KEY_B, VkB },
	{ XKB_KEY_N, VkN },
	{ XKB_KEY_M, VkM },
	{ XKB_KEY_1, Vk1 },
	{ XKB_KEY_2, Vk2 },
	{ XKB_KEY_3, Vk3 },
	{ XKB_KEY_4, Vk4 },
	{ XKB_KEY_5, Vk5 },
	{ XKB_KEY_6, Vk6 },
	{ XKB_KEY_7, Vk7 },
	{ XKB_KEY_8, Vk8 },
	{ XKB_KEY_9, Vk9 },
	{ XKB_KEY_0, Vk0 },
};

	}

VirtualKey translateToVirtualKey(xkb_keysym_t keySym)
{
	for (int32_t i = 0; i < sizeof_array(c_translateTable); ++i)
	{
		if (c_translateTable[i].keySym == keySym)
			return c_translateTable[i].vkey;
	}
	return VkNull;
}

}

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/keysymdef.h>
#include "Ui/X11/UtilitiesX11.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

const struct { uint32_t keySym; VirtualKey vkey; } c_translateTable[] =
{
	{ XK_Tab, VkTab },
	//{ VK_CLEAR, VkClear },
	{ XK_Return, VkReturn },
	{ XK_Shift_L, VkShift },
	{ XK_Shift_R, VkShift },
	{ XK_Control_L, VkControl },
	{ XK_Control_R, VkControl },
	{ XK_Meta_L, VkMenu },
	//{ VK_PAUSE, VkPause },
	//{ VK_CAPITAL, VkCapital },
	{ XK_Escape, VkEscape },
	{ XK_space, VkSpace },
	{ XK_End, VkEnd },
	{ XK_Home, VkHome },
	//{ VK_PRIOR, VkPageUp },
	//{ VK_NEXT, VkPageDown },
	{ XK_Left, VkLeft },
	{ XK_Up, VkUp },
	{ XK_Right, VkRight },
	{ XK_Down, VkDown },
	//{ VK_SELECT, VkSelect },
	//{ VK_PRINT, VkPrint },
	//{ VK_EXECUTE, VkExecute },
	//{ VK_SNAPSHOT, VkSnapshot },
	//{ VK_INSERT, VkInsert },
	{ XK_Delete, VkDelete },
	//{ VK_BACK, VkBackSpace },
	//{ VK_HELP, VkHelp },
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
	//{ VK_SCROLL, VkScroll }
};

		}

VirtualKey translateToVirtualKey(KeySym keySym)
{
	for (int32_t i = 0; i < sizeof_array(c_translateTable); ++i)
	{
		if (c_translateTable[i].keySym == keySym)
			return c_translateTable[i].vkey;
	}
	return VkNull;
}

	}
}
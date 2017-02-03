#include "Ui/Win32/UtilitiesWin32.h"
#include "Ui/Win32/Window.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

const struct { uint32_t keyCode; VirtualKey vkey; } c_translateTable[] =
{
	{ VK_TAB, VkTab },
	{ VK_CLEAR, VkClear },
	{ VK_RETURN, VkReturn },
	{ VK_SHIFT, VkShift },
	{ VK_CONTROL, VkControl },
	{ VK_MENU, VkMenu },
	{ VK_PAUSE, VkPause },
	{ VK_CAPITAL, VkCapital },
	{ VK_ESCAPE, VkEscape },
	{ VK_SPACE, VkSpace },
	{ VK_END, VkEnd },
	{ VK_HOME, VkHome },
	{ VK_PRIOR, VkPageUp },
	{ VK_NEXT, VkPageDown },
	{ VK_LEFT, VkLeft },
	{ VK_UP, VkUp },
	{ VK_RIGHT, VkRight },
	{ VK_DOWN, VkDown },
	{ VK_SELECT, VkSelect },
	{ VK_PRINT, VkPrint },
	{ VK_EXECUTE, VkExecute },
	{ VK_SNAPSHOT, VkSnapshot },
	{ VK_INSERT, VkInsert },
	{ VK_DELETE, VkDelete },
	{ VK_BACK, VkBackSpace },
	{ VK_HELP, VkHelp },
	{ VK_NUMPAD0, VkNumPad0 },
	{ VK_NUMPAD1, VkNumPad1 },
	{ VK_NUMPAD2, VkNumPad2 },
	{ VK_NUMPAD3, VkNumPad3 },
	{ VK_NUMPAD4, VkNumPad4 },
	{ VK_NUMPAD5, VkNumPad5 },
	{ VK_NUMPAD6, VkNumPad6 },
	{ VK_NUMPAD7, VkNumPad7 },
	{ VK_NUMPAD8, VkNumPad8 },
	{ VK_NUMPAD9, VkNumPad9 },
	{ VK_MULTIPLY, VkMultiply },
	{ VK_ADD, VkAdd },
	{ VK_SEPARATOR, VkSeparator },
	{ VK_SUBTRACT, VkSubtract },
	{ VK_DECIMAL, VkDecimal },
	{ VK_DIVIDE, VkDivide },
	{ VK_F1, VkF1 },
	{ VK_F2, VkF2 },
	{ VK_F3, VkF3 },
	{ VK_F4, VkF4 },
	{ VK_F5, VkF5 },
	{ VK_F6, VkF6 },
	{ VK_F7, VkF7 },
	{ VK_F8, VkF8 },
	{ VK_F9, VkF9 },
	{ VK_F10, VkF10 },
	{ VK_F11, VkF11 },
	{ VK_F12, VkF12 },
	{ VK_NUMLOCK, VkNumLock },
	{ VK_SCROLL, VkScroll }
};

		}

VirtualKey translateToVirtualKey(uint32_t keyCode)
{
	if ((keyCode >= '0' && keyCode <= '9') || (keyCode >= 'A' && keyCode <= 'Z'))
		return (VirtualKey)keyCode;
	for (int32_t i = 0; i < sizeof_array(c_translateTable); ++i)
	{
		if (c_translateTable[i].keyCode == keyCode)
			return c_translateTable[i].vkey;
	}
	return VkNull;
}

uint32_t translateToKeyCode(VirtualKey vk)
{
	if (vk >= VkA && vk <= VkZ)
		return (uint32_t)vk;
	if (vk >= Vk0 && vk <= Vk9)
		return (uint32_t)vk;
	for (int32_t i = 0; i < sizeof_array(c_translateTable); ++i)
	{
		if (c_translateTable[i].vkey == vk)
			return c_translateTable[i].keyCode;
	}
	return 0;
}
	
	}
}

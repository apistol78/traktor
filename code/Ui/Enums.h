/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"

#if defined(None)
#undef None
#endif

namespace traktor::ui
{

/*! \ingroup UI */
//@{

constexpr static uint32_t WsNone			= 0;
constexpr static uint32_t WsBorder			= 1 << 0;
constexpr static uint32_t WsClientBorder	= 1 << 1;
constexpr static uint32_t WsResizable		= 1 << 2;
constexpr static uint32_t WsSystemBox		= 1 << 3;
constexpr static uint32_t WsMinimizeBox		= 1 << 4;
constexpr static uint32_t WsMaximizeBox		= 1 << 5;
constexpr static uint32_t WsCloseBox		= 1 << 6;
constexpr static uint32_t WsCaption			= 1 << 7;
constexpr static uint32_t WsTop				= 1 << 8;
constexpr static uint32_t WsDoubleBuffer	= 1 << 9;
constexpr static uint32_t WsAccelerated		= 1 << 10;
constexpr static uint32_t WsAcceptFileDrop	= 1 << 11;
constexpr static uint32_t WsTabStop			= 1 << 12;
constexpr static uint32_t WsWantAllInput	= 1 << 13;
constexpr static uint32_t WsNoCanvas		= 1 << 14;	//!< Don't allocate a graphics canvas; useful for 3d rendering widgets etc.
constexpr static uint32_t WsFocus			= 1 << 15;	//!< Get focus when user press mouse on widget.
constexpr static uint32_t WsUser			= 1 << 16;

constexpr static uint32_t MbOk				= 1 << 0;
constexpr static uint32_t MbCancel			= 1 << 1;
constexpr static uint32_t MbYesNo			= 1 << 2;
constexpr static uint32_t MbNoDefault		= 1 << 3;
constexpr static uint32_t MbIconExclamation	= 1 << 4;
constexpr static uint32_t MbIconHand		= 1 << 5;
constexpr static uint32_t MbIconError		= 1 << 6;
constexpr static uint32_t MbIconQuestion	= 1 << 7;
constexpr static uint32_t MbIconInformation	= 1 << 8;
constexpr static uint32_t MbStayOnTop		= 1 << 9;

enum class DialogResult
{
	Cancel		= 0,
	Ok			= 1,
	Apply		= 2,
	No			= Cancel,
	Yes			= Ok
};

enum class ClipboardContent
{
	Empty,
	Object,
	Text,
	Image
};

enum class Cursor
{
	None,
	Arrow,
	ArrowRight,
	ArrowWait,
	Cross,
	Hand,
	IBeam,
	SizeNESW,
	SizeNS,
	SizeNWSE,
	SizeWE,
	Sizing,
	Wait
};

enum Align
{
	AnLeft,
	AnTop		= AnLeft,
	AnCenter,
	AnRight,
	AnBottom	= AnRight
};

enum class LineStyle
{
	Solid,
	Dot,
	DotDash
};

enum class BlendMode
{
	Opaque,
	Alpha,
	Modulate
};

enum class Filter
{
	Nearest,
	Linear
};

enum KeyStates
{
	KsNone		= 0,
	KsControl	= 1 << 0,
	KsMenu		= 1 << 1,
	KsShift		= 1 << 2,
	KsCommand	= 1 << 3,
	KsAll		= (KsControl | KsMenu | KsShift | KsCommand)
};

enum MouseButtons
{
	MbtNone		= 0,
	MbtLeft		= 1,
	MbtMiddle	= 2,
	MbtRight	= 4
};

/*! Virtual key codes.
 *
 * Platform independent key codes,
 * used for KeyEvent.
 */
enum VirtualKey
{
	VkNull = 0,

	VkTab		= 1000,
	VkClear		= 1001,
	VkReturn	= 1002,
	VkShift		= 1003,
	VkControl	= 1004,
	VkMenu		= 1005,
	VkPause		= 1006,
	VkCapital	= 1007,
	VkEscape	= 1008,
	VkSpace		= 1009,
	VkEnd		= 1010,
	VkHome		= 1011,
	VkPageUp	= 1012,
	VkPageDown	= 1013,
	VkLeft		= 1014,
	VkUp		= 1015,
	VkRight		= 1016,
	VkDown		= 1017,
	VkSelect	= 1018,
	VkPrint		= 1019,
	VkExecute	= 1020,
	VkSnapshot	= 1021,
	VkInsert	= 1022,
	VkDelete	= 1023,
	VkBackSpace	= 1024,
	VkHelp		= 1025,
	VkNumPad0	= 1026,
	VkNumPad1	= 1027,
	VkNumPad2	= 1028,
	VkNumPad3	= 1029,
	VkNumPad4	= 1030,
	VkNumPad5	= 1031,
	VkNumPad6	= 1032,
	VkNumPad7	= 1033,
	VkNumPad8	= 1034,
	VkNumPad9	= 1035,
	VkMultiply	= 1036,
	VkAdd		= 1037,
	VkSeparator	= 1038,
	VkSubtract	= 1039,
	VkDecimal	= 1040,
	VkDivide	= 1041,
	VkF1		= 1042,
	VkF2		= 1043,
	VkF3		= 1044,
	VkF4		= 1045,
	VkF5		= 1046,
	VkF6		= 1047,
	VkF7		= 1048,
	VkF8		= 1049,
	VkF9		= 1050,
	VkF10		= 1051,
	VkF11		= 1052,
	VkF12		= 1053,
	VkNumLock	= 1054,
	VkScroll	= 1055,

	VkComma = ',',
	VkPeriod = '.',

	Vk0 = '0',
	Vk1 = '1',
	Vk2 = '2',
	Vk3 = '3',
	Vk4 = '4',
	Vk5 = '5',
	Vk6 = '6',
	Vk7 = '7',
	Vk8 = '8',
	Vk9 = '9',
	VkA = 'A',
	VkB = 'B',
	VkC = 'C',
	VkD = 'D',
	VkE = 'E',
	VkF = 'F',
	VkG = 'G',
	VkH = 'H',
	VkI = 'I',
	VkJ = 'J',
	VkK = 'K',
	VkL = 'L',
	VkM = 'M',
	VkN = 'N',
	VkO = 'O',
	VkP = 'P',
	VkQ = 'Q',
	VkR = 'R',
	VkS = 'S',
	VkT = 'T',
	VkU = 'U',
	VkV = 'V',
	VkW = 'W',
	VkX = 'X',
	VkY = 'Y',
	VkZ = 'Z'
};

//@}

}

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_Enums_H
#define traktor_ui_Enums_H

namespace traktor
{
	namespace ui
	{

/*! \ingroup UI */
//@{

enum WidgetStyles
{
	WsNone				= 0,
	WsBorder			= 1 << 0,
	WsClientBorder		= 1 << 1,
	WsResizable			= 1 << 2,
	WsSystemBox			= 1 << 3,
	WsMinimizeBox		= 1 << 4,
	WsMaximizeBox		= 1 << 5,
	WsCloseBox			= 1 << 6,
	WsCaption			= 1 << 7,
	WsTop				= 1 << 8,
	WsDoubleBuffer		= 1 << 9,
	WsAccelerated		= 1 << 10,
	WsAcceptFileDrop	= 1 << 11,
	WsTabStop			= 1 << 12,
	WsWantAllInput		= 1 << 13,
	WsUser				= 1 << 14
};

enum MessageBoxStyles
{
	MbOk				= 1 << 0,
	MbCancel			= 1 << 1,
	MbYesNo				= 1 << 2,
	MbNoDefault			= 1 << 3,
	MbIconExclamation	= 1 << 4,
	MbIconHand			= 1 << 5,
	MbIconError			= 1 << 6,
	MbIconQuestion		= 1 << 7,
	MbIconInformation	= 1 << 8,
	MbStayOnTop			= 1 << 9
};

enum DialogResult
{
	DrCancel		= 0,
	DrOk			= 1,
	DrApply			= 2,
	DrNo			= DrCancel,
	DrYes			= DrOk
};

enum ClipboardContentType
{
	CtEmpty,
	CtObject,
	CtText,
	CtImage
};

enum Cursor
{
	CrNone,
	CrArrow,
	CrArrowRight,
	CrArrowWait,
	CrCross,
	CrHand,
	CrIBeam,
	CrSizeNESW,
	CrSizeNS,
	CrSizeNWSE,
	CrSizeWE,
	CrSizing,
	CrWait
};

enum Align
{
	AnLeft,
	AnTop		= AnLeft,
	AnCenter,
	AnRight,
	AnBottom	= AnRight
};

enum LineStyle
{
	LsSolid,
	LsDot,
	LsDotDash
};

enum SystemColor
{
	ScDesktopBackground,
	ScActiveCaption,
	ScInactiveCaption,
	ScMenuBackground,
	ScWindowBackground,
	ScWindowFrame,
	ScMenuText,
	ScWindowText,
	ScActiveCaptionText,
	ScInactiveCaptionText,
	ScActiveBorder,
	ScInactiveBorder,
	ScButtonFace,
	ScButtonShadow,
	ScDisabledText
};

enum BlendMode
{
	BmNone		= 0,
	BmAlpha		= 1,
	BmModulate	= 2
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

/*! \brief Virtual key codes.
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
}

#endif	// traktor_ui_Enums_H

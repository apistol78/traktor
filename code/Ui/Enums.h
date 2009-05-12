#ifndef traktor_ui_Enums_H
#define traktor_ui_Enums_H

#include "Core/Heap/Ref.h"

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
	WsUser				= 1 << 12
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
	BmNone,
	BmAlpha
};

enum KeyStates
{
	KsNone		= 0,
	KsControl	= 1 << 0,
	KsMenu		= 1 << 1,
	KsShift		= 1 << 2
};

enum EventId
{
	EiChild,
	EiKey,
	EiKeyDown,
	EiKeyUp,
	EiMove,
	EiSize,
	EiShow,
	EiButtonDown,
	EiButtonUp,
	EiClick,
	EiDoubleClick,
	EiMouseMove,
	EiMouseWheel,
	EiFocus,
	EiPaint,
	EiTimer,
	EiSelectionChange,
	EiClose,
	EiActivate,
	EiScroll,
	EiContentChange,
	EiCommand,
	EiColumnClick,
	EiNcButtonDown,
	EiNcButtonUp,
	EiNcDoubleClick,
	EiNcMouseMove,
	EiNcMouseWheel,
	EiFileDrop,
	EiIdle,
	EiUser
};

//@}

	}
}

#endif	// traktor_ui_Enums_H

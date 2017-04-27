/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_input_InputTypes_H
#define traktor_input_InputTypes_H

#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

/*! \brief Input control type.
 * \ingroup Input
 */
enum InputDefaultControlType
{
	DtInvalid			= 0,

	// Mouse, Joystick and Wheel
	DtUp				= 1,
	DtDown				= 2,
	DtLeft				= 3,
	DtRight				= 4,
	DtSelect			= 5,
	DtCancel			= 6,
	DtThumbLeftX		= 7,
	DtThumbLeftY		= 8,
	DtThumbLeftZ		= 9,
	DtThumbLeftPush		= 10,
	DtThumbRightX		= 11,
	DtThumbRightY		= 12,
	DtThumbRightZ		= 13,
	DtThumbRightPush	= 14,
	DtSliderLeft		= 15,
	DtSliderRight		= 16,
	DtTriggerLeft		= 17,
	DtTriggerRight		= 18,
	DtShoulderLeft		= 19,
	DtShoulderRight		= 20,
	DtButton1			= 21,
	DtButton2			= 22,
	DtButton3			= 23,
	DtButton4			= 24,
	DtButton5			= 25,
	DtButton6			= 26,
	DtButton7			= 27,
	DtButton8			= 28,
	DtReserved1			= 29,

	// Relative axises, ex mouse axises.
	DtAxisX				= 30,
	DtAxisY				= 31,
	DtAxisZ				= 32,

	// Absolute axises, ex mouse position, first touch.
	DtPositionX			= 33,
	DtPositionY			= 34,
	DtPositionZ			= 35,

	// Absolute axises, ex second and third touch.
	DtPositionX2		= 36,
	DtPositionY2		= 37,
	DtPositionX3		= 38,
	DtPositionY3		= 39,
	DtPositionX4		= 200,
	DtPositionY4		= 201,

	// Keyboard
	DtKeyEscape			= 40,
	DtKey1				= 41,
	DtKey2				= 42,
	DtKey3				= 43,
	DtKey4				= 44,
	DtKey5				= 45,
	DtKey6				= 46,
	DtKey7				= 47,
	DtKey8				= 48,
	DtKey9				= 49,
	DtKey0				= 50,
	DtKeyMinus			= 51,
	DtKeyEquals			= 52,
	DtKeyBack			= 53,
	DtKeyTab			= 54,
	DtKeyQ				= 55,
	DtKeyW				= 56,
	DtKeyE				= 57,
	DtKeyR				= 58,
	DtKeyT				= 59,
	DtKeyY				= 60,
	DtKeyU				= 61,
	DtKeyI				= 62,
	DtKeyO				= 63,
	DtKeyP				= 64,
	DtKeyLeftBracket	= 65,
	DtKeyRightBracket	= 66,
	DtKeyReturn			= 67,
	DtKeyLeftControl	= 68,
	DtKeyA				= 69,
	DtKeyS				= 70,
	DtKeyD				= 71,
	DtKeyF				= 72,
	DtKeyG				= 73,
	DtKeyH				= 74,
	DtKeyJ				= 75,
	DtKeyK				= 76,
	DtKeyL				= 77,
	DtKeySemicolon		= 78,
	DtKeyApostrophe		= 79,
	DtKeyGrave			= 80,
	DtKeyLeftShift		= 81,
	DtKeyBackSlash		= 82,
	DtKeyZ				= 83,
	DtKeyX				= 84,
	DtKeyC				= 85,
	DtKeyV				= 86,
	DtKeyB				= 87,
	DtKeyN				= 88,
	DtKeyM				= 89,
	DtKeyComma			= 90,
	DtKeyPeriod			= 91,
	DtKeySlash			= 92,
	DtKeyRightShift		= 93,
	DtKeyMultiply		= 94,
	DtKeyLeftMenu		= 95,
	DtKeySpace			= 96,
	DtKeyCaptial		= 97,
	DtKeyF1				= 98,
	DtKeyF2				= 99,
	DtKeyF3				= 100,
	DtKeyF4				= 101,
	DtKeyF5				= 102,
	DtKeyF6				= 103,
	DtKeyF7				= 104,
	DtKeyF8				= 105,
	DtKeyF9				= 106,
	DtKeyF10			= 107,
	DtKeyNumLock		= 108,
	DtKeyScroll			= 109,
	DtKeyNumPad7		= 110,
	DtKeyNumPad8		= 111,
	DtKeyNumPad9		= 112,
	DtKeySubtract		= 113,
	DtKeyNumPad4		= 114,
	DtKeyNumPad5		= 115,
	DtKeyNumPad6		= 116,
	DtKeyAdd			= 117,
	DtKeyNumPad1		= 118,
	DtKeyNumPad2		= 119,
	DtKeyNumPad3		= 120,
	DtKeyNumPad0		= 121,
	DtKeyDecimal		= 122,
	DtKeyOem102			= 123,
	DtKeyF11			= 124,
	DtKeyF12			= 125,
	DtKeyF13			= 126,
	DtKeyF14			= 127,
	DtKeyF15			= 128,
	DtKeyKana			= 129,
	DtKeyAbntC1			= 130,
	DtKeyConvert		= 131,
	DtKeyNoConvert		= 132,
	DtKeyYen			= 133,
	DtKeyAbntC2			= 134,
	DtKeyNumPadEquals	= 135,
	DtKeyPreviousTrack	= 136,
	DtKeyAt				= 137,
	DtKeyColon			= 138,
	DtKeyUnderline		= 139,
	DtKeyKanji			= 140,
	DtKeyStop			= 141,
	DtKeyAx				= 142,
	DtKeyUnlabeled		= 143,
	DtKeyNextTrack		= 144,
	DtKeyNumPadEnter	= 145,
	DtKeyRightControl	= 146,
	DtKeyMute			= 147,
	DtKeyCalculator		= 148,
	DtKeyPlayPause		= 149,
	DtKeyMediaStop		= 150,
	DtKeyVolumeDown		= 151,
	DtKeyVolumeUp		= 152,
	DtKeyWebHome		= 153,
	DtKeyNumPadComma	= 154,
	DtKeyDivide			= 155,
	DtKeySysRq			= 156,
	DtKeyRightMenu		= 157,
	DtKeyPause			= 158,
	DtKeyHome			= 159,
	DtKeyUp				= 160,
	DtKeyPrior			= 161,
	DtKeyLeft			= 162,
	DtKeyRight			= 163,
	DtKeyEnd			= 164,
	DtKeyDown			= 165,
	DtKeyNext			= 166,
	DtKeyInsert			= 167,
	DtKeyDelete			= 168,
	DtKeyLeftWin		= 169,
	DtKeyRightWin		= 170,
	DtKeyApplications	= 171,
	DtKeyPower			= 172,
	DtKeySleep			= 173,
	DtKeyWake			= 174,
	DtKeyWebSearch		= 175,
	DtKeyWebFavorites	= 176,
	DtKeyWebRefresh		= 177,
	DtKeyWebStop		= 178,
	DtKeyWebForward		= 179,
	DtKeyWebBack		= 180,
	DtKeyMyComputer		= 181,
	DtKeyMail			= 182,
	DtKeyMediaSelect	= 183,

	// First and last key index.
	DtKeyFirstIndex = DtKeyEscape,
	DtKeyLastIndex = DtKeyMediaSelect
};

/*! \brief Input device category.
 * \ingroup Input
 */
enum InputCategory
{
	CtKeyboard		= 1,
	CtMouse			= 2,
	CtJoystick		= 4,
	CtWheel			= 8,
	CtTouch			= 16,
	CtGaze			= 32,
	CtAcceleration	= 64,
	CtOrientation	= 128,
	CtUnknown		= 256
};

/*! \brief Input rumble specification.
 * \ingroup Input
 */
struct InputRumble
{
	float lowFrequencyRumble;
	float highFrequencyRumble;
};

/*! \brief Shader parameter handle. */
typedef uint32_t handle_t;

/*! \brief Return handle from parameter name.
 *
 * \param name Parameter name.
 * \return Parameter handle.
 */
handle_t T_DLLCLASS getParameterHandle(const std::wstring& name);

	}
}

#endif	// traktor_input_InputTypes_H

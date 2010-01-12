#ifndef traktor_input_InputTypes_H
#define traktor_input_InputTypes_H

namespace traktor
{
	namespace input
	{

/*! \brief Input control type.
 * \ingroup Input
 */
enum InputDefaultControlType
{
	DtInvalid		= 0,

	// Mouse, Joystick and Wheel
	DtUp			= 1,
	DtDown			= 2,
	DtLeft			= 3,
	DtRight			= 4,
	DtSelect		= 5,
	DtCancel		= 6,
	DtThumbLeftX	= 7,
	DtThumbLeftY	= 8,
	DtThumbLeftPush	= 9,
	DtThumbRightX	= 10,
	DtThumbRightY	= 11,
	DtThumbRightPush	= 12,
	DtTriggerLeft		= 13,
	DtTriggerRight		= 14,
	DtShoulderLeft		= 15,
	DtShoulderRight		= 16,
	DtButton1		= 17,
	DtButton2		= 18,
	DtButton3		= 19,
	DtButton4		= 20,
	DtAxisX			= 21,
	DtAxisY			= 22,

	// Keyboard
	DtKeyEscape		= 23,
	DtKey1			= 24,
	DtKey2			= 25,
	DtKey3			= 26,
	DtKey4			= 27,
	DtKey5			= 28,
	DtKey6			= 29,
	DtKey7			= 30,
	DtKey8			= 31,
	DtKey9			= 32,
	DtKey0			= 33,
	DtKeyMinus		= 34,
	DtKeyEquals		= 35,
	DtKeyBack		= 36,
	DtKeyTab		= 37,
	DtKeyQ			= 38,
	DtKeyW			= 39,
	DtKeyE			= 40,
	DtKeyR			= 41,
	DtKeyT			= 42,
	DtKeyY			= 43,
	DtKeyU			= 44,
	DtKeyI			= 45,
	DtKeyO			= 46,
	DtKeyP			= 47,
	DtKeyLeftBracket	= 48,
	DtKeyRightBracket	= 49,
	DtKeyReturn		= 50,
	DtKeyLeftControl	= 51,
	DtKeyA			= 52,
	DtKeyS			= 53,
	DtKeyD			= 54,
	DtKeyF			= 55,
	DtKeyG			= 56,
	DtKeyH			= 57,
	DtKeyJ			= 58,
	DtKeyK			= 59,
	DtKeyL			= 60,
	DtKeySemicolon	= 61,
	DtKeyApostrophe	= 62,
	DtKeyGrave		= 63,
	DtKeyLeftShift	= 64,
	DtKeyBackSlash	= 65,
	DtKeyZ			= 66,
	DtKeyX			= 67,
	DtKeyC			= 68,
	DtKeyV			= 69,
	DtKeyB			= 70,
	DtKeyN			= 71,
	DtKeyM			= 72,
	DtKeyComma		= 73,
	DtKeyPeriod		= 74,
	DtKeySlash		= 75,
	DtKeyRightShift	= 76,
	DtKeyMultiply	= 77,
	DtKeyLeftMenu	= 78,
	DtKeySpace		= 79,
	DtKeyCaptial	= 80,
	DtKeyF1			= 81,
	DtKeyF2			= 82,
	DtKeyF3			= 83,
	DtKeyF4			= 84,
	DtKeyF5			= 85,
	DtKeyF6			= 86,
	DtKeyF7			= 87,
	DtKeyF8			= 88,
	DtKeyF9			= 89,
	DtKeyF10		= 90,
	DtKeyNumLock	= 91,
	DtKeyScroll		= 92,
	DtKeyNumPad7	= 93,
	DtKeyNumPad8	= 94,
	DtKeyNumPad9	= 95,
	DtKeySubtract	= 96,
	DtKeyNumPad4	= 97,
	DtKeyNumPad5	= 98,
	DtKeyNumPad6	= 99,
	DtKeyAdd		= 100,
	DtKeyNumPad1	= 101,
	DtKeyNumPad2	= 102,
	DtKeyNumPad3	= 103,
	DtKeyNumPad0	= 104,
	DtKeyDecimal	= 105,
	DtKeyOem102		= 106,
	DtKeyF11		= 107,
	DtKeyF12		= 108,
	DtKeyF13		= 109,
	DtKeyF14		= 110,
	DtKeyF15		= 111,
	DtKeyKana		= 112,
	DtKeyAbntC1		= 113,
	DtKeyConvert	= 114,
	DtKeyNoConvert	= 115,
	DtKeyYen		= 116,
	DtKeyAbntC2		= 117,
	DtKeyNumPadEquals	= 118,
	DtKeyPreviousTrack	= 119,
	DtKeyAt			= 120,
	DtKeyColon		= 121,
	DtKeyUnderline	= 122,
	DtKeyKanji		= 123,
	DtKeyStop		= 124,
	DtKeyAx			= 125,
	DtKeyUnlabeled	= 126,
	DtKeyNextTrack	= 127,
	DtKeyNumPadEnter	= 128,
	DtKeyRightControl	= 129,
	DtKeyMute		= 130,
	DtKeyCalculator	= 131,
	DtKeyPlayPause	= 132,
	DtKeyMediaStop	= 133,
	DtKeyVolumeDown	= 134,
	DtKeyVolumeUp	= 135,
	DtKeyWebHome	= 136,
	DtKeyNumPadComma	= 137,
	DtKeyDivide		= 138,
	DtKeySysRq		= 139,
	DtKeyRightMenu	= 140,
	DtKeyPause		= 141,
	DtKeyHome		= 142,
	DtKeyUp			= 143,
	DtKeyPrior		= 144,
	DtKeyLeft		= 145,
	DtKeyRight		= 146,
	DtKeyEnd		= 147,
	DtKeyDown		= 148,
	DtKeyNext		= 149,
	DtKeyInsert		= 150,
	DtKeyDelete		= 151,
	DtKeyLeftWin	= 152,
	DtKeyRightWin	= 153,
	DtKeyApplications	= 154,
	DtKeyPower		= 155,
	DtKeySleep		= 156,
	DtKeyWake		= 157,
	DtKeyWebSearch	= 158,
	DtKeyWebFavorites	= 159,
	DtKeyWebRefresh	= 160,
	DtKeyWebStop	= 161,
	DtKeyWebForward	= 162,
	DtKeyWebBack	= 163,
	DtKeyMyComputer	= 164,
	DtKeyMail		= 165,
	DtKeyMediaSelect	= 166,

	// First and last key index.
	DtKeyFirstIndex = DtKeyEscape,
	DtKeyLastIndex = DtKeyMediaSelect
};

/*! \brief Input device category.
 * \ingroup Input
 */
enum InputCategory
{
	CtKeyboard	= 1,
	CtMouse		= 2,
	CtJoystick	= 4,
	CtWheel		= 8,
	CtUnknown	= 16
};

/*! \brief Input rumble specification.
 * \ingroup Input
 */
struct InputRumble
{
	float lowFrequencyRumble;
	float highFrequencyRumble;
};

	}
}

#endif	// traktor_input_InputTypes_H

/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::input
{

/*! Input control type.
 * \ingroup Input
 */
enum class DefaultControl
{
	Invalid				= 0,

	// Mouse, Joystick and Wheel
	Up					= 1,
	Down				= 2,
	Left				= 3,
	Right				= 4,
	Select				= 5,
	Cancel				= 6,
	ThumbLeftX			= 7,
	ThumbLeftY			= 8,
	ThumbLeftZ			= 9,
	ThumbLeftPush		= 10,
	ThumbRightX			= 11,
	ThumbRightY			= 12,
	ThumbRightZ			= 13,
	ThumbRightPush		= 14,
	SliderLeft			= 15,
	SliderRight			= 16,
	TriggerLeft			= 17,
	TriggerRight		= 18,
	ShoulderLeft		= 19,
	ShoulderRight		= 20,
	Button1				= 21,
	Button2				= 22,
	Button3				= 23,
	Button4				= 24,
	Button5				= 25,
	Button6				= 26,
	Button7				= 27,
	Button8				= 28,
	Reserved1			= 29,

	// Relative axises, ex mouse axises.
	AxisX				= 30,
	AxisY				= 31,
	AxisZ				= 32,

	// Absolute axises, ex mouse position, first touch.
	PositionX			= 33,
	PositionY			= 34,
	PositionZ			= 35,

	// Absolute axises, ex second and third touch.
	PositionX2			= 36,
	PositionY2			= 37,
	PositionX3			= 38,
	PositionY3			= 39,
	PositionX4			= 200,
	PositionY4			= 201,

	// Keyboard
	KeyEscape			= 40,
	Key1				= 41,
	Key2				= 42,
	Key3				= 43,
	Key4				= 44,
	Key5				= 45,
	Key6				= 46,
	Key7				= 47,
	Key8				= 48,
	Key9				= 49,
	Key0				= 50,
	KeyMinus			= 51,
	KeyEquals			= 52,
	KeyBack				= 53,
	KeyTab				= 54,
	KeyQ				= 55,
	KeyW				= 56,
	KeyE				= 57,
	KeyR				= 58,
	KeyT				= 59,
	KeyY				= 60,
	KeyU				= 61,
	KeyI				= 62,
	KeyO				= 63,
	KeyP				= 64,
	KeyLeftBracket		= 65,
	KeyRightBracket		= 66,
	KeyReturn			= 67,
	KeyLeftControl		= 68,
	KeyA				= 69,
	KeyS				= 70,
	KeyD				= 71,
	KeyF				= 72,
	KeyG				= 73,
	KeyH				= 74,
	KeyJ				= 75,
	KeyK				= 76,
	KeyL				= 77,
	KeySemicolon		= 78,
	KeyApostrophe		= 79,
	KeyGrave			= 80,
	KeyLeftShift		= 81,
	KeyBackSlash		= 82,
	KeyZ				= 83,
	KeyX				= 84,
	KeyC				= 85,
	KeyV				= 86,
	KeyB				= 87,
	KeyN				= 88,
	KeyM				= 89,
	KeyComma			= 90,
	KeyPeriod			= 91,
	KeySlash			= 92,
	KeyRightShift		= 93,
	KeyMultiply			= 94,
	KeyLeftMenu			= 95,
	KeySpace			= 96,
	KeyCaptial			= 97,
	KeyF1				= 98,
	KeyF2				= 99,
	KeyF3				= 100,
	KeyF4				= 101,
	KeyF5				= 102,
	KeyF6				= 103,
	KeyF7				= 104,
	KeyF8				= 105,
	KeyF9				= 106,
	KeyF10				= 107,
	KeyNumLock			= 108,
	KeyScroll			= 109,
	KeyNumPad7			= 110,
	KeyNumPad8			= 111,
	KeyNumPad9			= 112,
	KeySubtract			= 113,
	KeyNumPad4			= 114,
	KeyNumPad5			= 115,
	KeyNumPad6			= 116,
	KeyAdd				= 117,
	KeyNumPad1			= 118,
	KeyNumPad2			= 119,
	KeyNumPad3			= 120,
	KeyNumPad0			= 121,
	KeyDecimal			= 122,
	KeyOem102			= 123,
	KeyF11				= 124,
	KeyF12				= 125,
	KeyF13				= 126,
	KeyF14				= 127,
	KeyF15				= 128,
	KeyKana				= 129,
	KeyAbntC1			= 130,
	KeyConvert			= 131,
	KeyNoConvert		= 132,
	KeyYen				= 133,
	KeyAbntC2			= 134,
	KeyNumPadEquals		= 135,
	KeyPreviousTrack	= 136,
	KeyAt				= 137,
	KeyColon			= 138,
	KeyUnderline		= 139,
	KeyKanji			= 140,
	KeyStop				= 141,
	KeyAx				= 142,
	KeyUnlabeled		= 143,
	KeyNextTrack		= 144,
	KeyNumPadEnter		= 145,
	KeyRightControl		= 146,
	KeyMute				= 147,
	KeyCalculator		= 148,
	KeyPlayPause		= 149,
	KeyMediaStop		= 150,
	KeyVolumeDown		= 151,
	KeyVolumeUp			= 152,
	KeyWebHome			= 153,
	KeyNumPadComma		= 154,
	KeyDivide			= 155,
	KeySysRq			= 156,
	KeyRightMenu		= 157,
	KeyPause			= 158,
	KeyHome				= 159,
	KeyUp				= 160,
	KeyPrior			= 161,
	KeyLeft				= 162,
	KeyRight			= 163,
	KeyEnd				= 164,
	KeyDown				= 165,
	KeyNext				= 166,
	KeyInsert			= 167,
	KeyDelete			= 168,
	KeyLeftWin			= 169,
	KeyRightWin			= 170,
	KeyApplications		= 171,
	KeyPower			= 172,
	KeySleep			= 173,
	KeyWake				= 174,
	KeyWebSearch		= 175,
	KeyWebFavorites		= 176,
	KeyWebRefresh		= 177,
	KeyWebStop			= 178,
	KeyWebForward		= 179,
	KeyWebBack			= 180,
	KeyMyComputer		= 181,
	KeyMail				= 182,
	KeyMediaSelect		= 183,

	// First and last key index.
	KeyFirstIndex		= KeyEscape,
	KeyLastIndex		= KeyMediaSelect
};

/*! Input device category.
 * \ingroup Input
 */
enum class InputCategory : uint32_t
{
	Invalid = 0,
	Keyboard = 1,
	Mouse = 2,
	Joystick = 4,
	Wheel = 8,
	Touch = 16,
	Gaze = 32,
	Acceleration = 64,
	Orientation = 128,
	Unknown = 256
};

constexpr InputCategory operator | (InputCategory lh, InputCategory rh) { return (InputCategory)((uint32_t)lh | (uint32_t)rh); }
constexpr InputCategory operator & (InputCategory lh, InputCategory rh) { return (InputCategory)((uint32_t)lh & (uint32_t)rh); }

/*! Input rumble specification.
 * \ingroup Input
 */
struct InputRumble
{
	float lowFrequencyRumble;
	float highFrequencyRumble;
};

/*! Shader parameter handle. */
typedef uint32_t handle_t;

/*! Return handle from parameter name.
 *
 * \param name Parameter name.
 * \return Parameter handle.
 */
handle_t T_DLLCLASS getParameterHandle(const std::wstring& name);

}

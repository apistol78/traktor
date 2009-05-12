#ifndef traktor_input_InputTypes_H
#define traktor_input_InputTypes_H

namespace traktor
{
	namespace input
	{

enum InputDefaultControlType
{
	DtUp,
	DtDown,
	DtLeft,
	DtRight,
	DtSelect,
	DtCancel,
	DtThumbLeftX,
	DtThumbLeftY,
	DtThumbLeftPush,
	DtThumbRightX,
	DtThumbRightY,
	DtThumbRightPush,
	DtTriggerLeft,
	DtTriggerRight,
	DtShoulderLeft,
	DtShoulderRight,
	DtButton1,
	DtButton2,
	DtButton3,
	DtButton4,
	DtAxisX,
	DtAxisY
};

enum InputCategory
{
	CtKeyboard,
	CtMouse,
	CtJoystick,
	CtWheel,
	CtUnknown
};

struct InputRumble
{
	float lowFrequencyRumble;
	float highFrequencyRumble;
};

	}
}

#endif	// traktor_input_InputTypes_H

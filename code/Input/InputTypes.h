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
	DtInvalid,
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

/*! \brief Input device category.
 * \ingroup Input
 */
enum InputCategory
{
	CtKeyboard,
	CtMouse,
	CtJoystick,
	CtWheel,
	CtUnknown
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

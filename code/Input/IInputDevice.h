#ifndef traktor_input_IInputDevice_H
#define traktor_input_IInputDevice_H

#include <string>
#include "Core/Object.h"
#include "Input/InputTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

/*! \brief Input device base.
 * \ingroup Input
 */
class T_DLLCLASS IInputDevice : public Object
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getName() const = 0;

	virtual InputCategory getCategory() const = 0;

	virtual bool isConnected() const = 0;

	virtual int getControlCount() = 0;

	virtual std::wstring getControlName(int control) = 0;

	virtual bool isControlAnalogue(int control) const = 0;

	virtual float getControlValue(int control) = 0;

	virtual bool getDefaultControl(InputDefaultControlType controlType, int& control) const = 0;

	virtual void resetState() = 0;

	virtual void readState() = 0;

	virtual bool supportRumble() const = 0;

	virtual void setRumble(const InputRumble& rumble) = 0;
};

	}
}

#endif	// traktor_input_IInputDevice_H

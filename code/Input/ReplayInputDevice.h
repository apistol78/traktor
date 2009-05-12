#ifndef traktor_input_ReplayInputDevice_H
#define traktor_input_ReplayInputDevice_H

#include "Core/Heap/Ref.h"
#include "Input/InputDevice.h"

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

class RecordInputScript;

class T_DLLCLASS ReplayInputDevice : public InputDevice
{
	T_RTTI_CLASS(ReplayInputDevice)

public:
	ReplayInputDevice(InputDevice* inputDevice, RecordInputScript* inputScript, bool loop);

	virtual std::wstring getName() const;

	virtual InputCategory getCategory() const;

	virtual int getControlCount();

	virtual std::wstring getControlName(int control);

	virtual bool isControlAnalogue(int control) const;

	virtual float getControlValue(int control);

	virtual bool getDefaultControl(InputDefaultControlType controlType, int& control) const;

	virtual void resetState();

	virtual void readState();

	virtual bool supportRumble() const;

	virtual void setRumble(const InputRumble& rumble);

private:
	Ref< InputDevice > m_inputDevice;
	Ref< RecordInputScript > m_inputScript;
	bool m_loop;
	uint32_t m_frame;
};

	}
}

#endif	// traktor_input_ReplayInputDevice_H

#ifndef traktor_input_RecordInputDevice_H
#define traktor_input_RecordInputDevice_H

#include "Core/Heap/Ref.h"
#include "Input/IInputDevice.h"

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

/*! \brief Recording input device.
 * \ingroup Input
 */
class T_DLLCLASS RecordInputDevice : public IInputDevice
{
	T_RTTI_CLASS(RecordInputDevice)

public:
	RecordInputDevice(IInputDevice* inputDevice, RecordInputScript* inputScript);

	virtual std::wstring getName() const;

	virtual InputCategory getCategory() const;

	virtual bool isConnected() const;

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
	Ref< IInputDevice > m_inputDevice;
	Ref< RecordInputScript > m_inputScript;
	uint32_t m_frame;
};

	}
}

#endif	// traktor_input_RecordInputDevice_H

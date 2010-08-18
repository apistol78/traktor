#ifndef traktor_input_ReplayInputDevice_H
#define traktor_input_ReplayInputDevice_H

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

/*! \brief Replay input device.
 * \ingroup Input
 */
class T_DLLCLASS ReplayInputDevice : public IInputDevice
{
	T_RTTI_CLASS;

public:
	ReplayInputDevice(IInputDevice* inputDevice, RecordInputScript* inputScript, bool loop);

	virtual std::wstring getName() const;

	virtual InputCategory getCategory() const;

	virtual bool isConnected() const;

	virtual int32_t getControlCount();

	virtual std::wstring getControlName(int32_t control);

	virtual bool isControlAnalogue(int32_t control) const;

	virtual float getControlValue(int32_t control);

	virtual bool getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const;

	virtual void resetState();

	virtual void readState();

	virtual bool supportRumble() const;

	virtual void setRumble(const InputRumble& rumble);

private:
	Ref< IInputDevice > m_inputDevice;
	Ref< RecordInputScript > m_inputScript;
	bool m_loop;
	uint32_t m_frame;
};

	}
}

#endif	// traktor_input_ReplayInputDevice_H

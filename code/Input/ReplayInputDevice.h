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

	virtual std::wstring getName() const T_OVERRIDE T_FINAL;

	virtual InputCategory getCategory() const T_OVERRIDE T_FINAL;

	virtual bool isConnected() const T_OVERRIDE T_FINAL;

	virtual int32_t getControlCount() T_OVERRIDE T_FINAL;

	virtual std::wstring getControlName(int32_t control) T_OVERRIDE T_FINAL;

	virtual bool isControlAnalogue(int32_t control) const T_OVERRIDE T_FINAL;

	virtual float getControlValue(int32_t control) T_OVERRIDE T_FINAL;

	virtual bool getControlRange(int32_t control, float& outMin, float& outMax) const T_OVERRIDE T_FINAL;

	virtual bool getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const T_OVERRIDE T_FINAL;

	virtual bool getKeyEvent(KeyEvent& outEvent) T_OVERRIDE T_FINAL;

	virtual void resetState() T_OVERRIDE T_FINAL;

	virtual void readState() T_OVERRIDE T_FINAL;

	virtual bool supportRumble() const T_OVERRIDE T_FINAL;

	virtual void setRumble(const InputRumble& rumble) T_OVERRIDE T_FINAL;

private:
	Ref< IInputDevice > m_inputDevice;
	Ref< RecordInputScript > m_inputScript;
	bool m_loop;
	uint32_t m_frame;
};

	}
}

#endif	// traktor_input_ReplayInputDevice_H

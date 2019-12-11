#pragma once

#include "Core/RefArray.h"
#include "Input/Binding/IInputSource.h"

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

class DeviceControl;
class DeviceControlManager;
class GenericInputSourceData;
class IInputDevice;
class InputSystem;

/*! Generic input source.
 * \ingroup Input
 */
class T_DLLCLASS GenericInputSource : public IInputSource
{
	T_RTTI_CLASS;

public:
	GenericInputSource(const GenericInputSourceData* data, DeviceControlManager* deviceControlManager);

	virtual std::wstring getDescription() const override final;

	virtual void prepare(float T, float dT) override final;

	virtual float read(float T, float dT) override final;

private:
	Ref< const GenericInputSourceData > m_data;
	Ref< DeviceControlManager > m_deviceControlManager;
	RefArray< DeviceControl > m_deviceControls;
	int32_t m_matchingDeviceCount;
	float m_lastValue;
};

	}
}


#ifndef traktor_input_GenericInputSource_H
#define traktor_input_GenericInputSource_H

#include <list>
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

class GenericInputSourceData;
class IInputDevice;
class InputSystem;

class T_DLLCLASS GenericInputSource : public IInputSource
{
	T_RTTI_CLASS;

public:
	GenericInputSource(const GenericInputSourceData* data);
	
	virtual std::wstring getDescription() const;

	virtual float read(InputSystem* inputSystem, float T, float dT);
	
private:
	struct DeviceControl
	{
		Ref< IInputDevice > device;
		int32_t control;
		float previousValue;
		float currentValue;
	};

	Ref< const GenericInputSourceData > m_data;
	std::list< DeviceControl > m_deviceControls;
	float m_lastValue;
};

	}
}

#endif	// traktor_input_GenericInputSource_H

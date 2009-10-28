#ifndef traktor_input_InputDriverIPhone_H
#define traktor_input_InputDriverIPhone_H

#include "Input/IInputDriver.h"
#include "Core/Heap/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_IPHONE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{
	
class InputDeviceTouch;
	
class T_DLLCLASS InputDriverIPhone : public IInputDriver
{
	T_RTTI_CLASS(InputDriverIPhone)
	
public:
	InputDriverIPhone();
	
	bool create(void* nativeWindowHandle);
	
	virtual int getDeviceCount();
	
	virtual IInputDevice* getDevice(int index);

private:
	Ref< InputDeviceTouch > m_inputDeviceTouch;
};
	
	}
}

#endif	// traktor_input_InputDriverIPhone_H

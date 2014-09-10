#ifndef traktor_input_InputDriveriOS_H
#define traktor_input_InputDriveriOS_H

#include "Core/Platform.h"
#include "Input/IInputDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_IOS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class InputDriveriOSImpl;
	
class T_DLLCLASS InputDriveriOS : public IInputDriver
{
	T_RTTI_CLASS;
	
public:
	InputDriveriOS();
	
	virtual bool create(void* nativeHandle, const SystemWindow& systemWindow, uint32_t inputCategories);
	
	virtual int getDeviceCount();
	
	virtual Ref< IInputDevice > getDevice(int index);

	virtual UpdateResult update();

private:
	Ref< InputDriveriOSImpl > m_impl;
};
	
	}
}

#endif	// traktor_input_InputDriveriOS_H

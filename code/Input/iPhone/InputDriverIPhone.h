#ifndef traktor_input_InputDriverIPhone_H
#define traktor_input_InputDriverIPhone_H

#include "Core/Platform.h"
#include "Input/IInputDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_IPHONE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class InputDriverIPhoneImpl;
	
class T_DLLCLASS InputDriverIPhone : public IInputDriver
{
	T_RTTI_CLASS;
	
public:
	InputDriverIPhone();
	
	virtual bool create(const SystemWindow& systemWindow, uint32_t inputCategories);
	
	virtual int getDeviceCount();
	
	virtual Ref< IInputDevice > getDevice(int index);

	virtual UpdateResult update();

private:
	Ref< InputDriverIPhoneImpl > m_impl;
};
	
	}
}

#endif	// traktor_input_InputDriverIPhone_H

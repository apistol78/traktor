#ifndef traktor_input_InputDriver_H
#define traktor_input_InputDriver_H

#include "Core/Object.h"

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

class InputDevice;

/*! \brief Input driver base.
 * \ingroup Input
 */
class T_DLLCLASS InputDriver : public Object
{
	T_RTTI_CLASS(InputDriver)

public:
	virtual int getDeviceCount() = 0;

	virtual InputDevice* getDevice(int index) = 0;
};

	}
}

#endif	// traktor_input_InputDriver_H

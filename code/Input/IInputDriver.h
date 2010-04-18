#ifndef traktor_input_IInputDriver_H
#define traktor_input_IInputDriver_H

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

class IInputDevice;

/*! \brief Input driver base.
 * \ingroup Input
 */
class T_DLLCLASS IInputDriver : public Object
{
	T_RTTI_CLASS;

public:
	enum UpdateResult
	{
		UrOk = 0,
		UrDevicesChanged = 1,
		UrFailure = -1
	};
	
	virtual int getDeviceCount() = 0;

	virtual Ref< IInputDevice > getDevice(int index) = 0;
	
	virtual UpdateResult update() = 0;
};

	}
}

#endif	// traktor_input_IInputDriver_H

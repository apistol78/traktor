/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_input_InputSystem_H
#define traktor_input_InputSystem_H

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Input/InputTypes.h"

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

class IInputDriver;
class IInputDevice;

/*! \brief Input system.
 * \ingroup Input
 */
class T_DLLCLASS InputSystem : public Object
{
	T_RTTI_CLASS;

public:
	void addDriver(IInputDriver* inputDriver);

	void removeDriver(IInputDriver* inputDriver);

	void addDevice(IInputDevice* inputDevice);

	void removeDevice(IInputDevice* inputDevice);

	int32_t getDeviceCount() const;

	Ref< IInputDevice > getDevice(int32_t index);

	int32_t getDeviceCount(InputCategory category, bool connected) const;

	Ref< IInputDevice > getDevice(InputCategory category, int32_t index, bool connected);

	void setExclusive(bool exclusive);

	bool update();

private:
	RefArray< IInputDriver > m_drivers;
	RefArray< IInputDevice > m_devices;

	void updateDevices();
};

	}
}

#endif	// traktor_input_InputSystem_H

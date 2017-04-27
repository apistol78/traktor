/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_input_InputDriverWin32_H
#define traktor_input_InputDriverWin32_H

#include "Core/RefArray.h"
#include "Input/IInputDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_WIN32_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

struct SystemWindow;

	namespace input
	{

class KeyboardDeviceWin32;
class MouseDeviceWin32;

class T_DLLCLASS InputDriverWin32 : public IInputDriver
{
	T_RTTI_CLASS;

public:
	InputDriverWin32();

	virtual bool create(const SystemApplication& sysapp, const SystemWindow& syswin, uint32_t inputCategories) T_OVERRIDE T_FINAL;

	virtual int getDeviceCount() T_OVERRIDE T_FINAL;

	virtual Ref< IInputDevice > getDevice(int index) T_OVERRIDE T_FINAL;

	virtual UpdateResult update() T_OVERRIDE T_FINAL;

private:
	Ref< KeyboardDeviceWin32 > m_keyboardDevice;
	Ref< MouseDeviceWin32 > m_mouseDevice;
	RefArray< IInputDevice > m_devices;
};

	}
}

#endif	// traktor_input_InputDriverWin32_H

/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes/BoxedStdVector.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Input/IInputDevice.h"
#include "Input/IInputDriver.h"
#include "Input/InputClassFactory.h"
#include "Input/InputSystem.h"
#include "Input/RumbleEffect.h"
#include "Input/RumbleEffectPlayer.h"
#include "Input/Binding/EnumKeys.h"
#include "Input/Binding/IInputSource.h"
#include "Input/Binding/InputMapping.h"
#include "Input/Binding/InputState.h"

namespace traktor::input
{
	namespace
	{

handle_t anyHandle(const Any& id)
{
	if (CastAny< handle_t >::accept(id))
		return CastAny< handle_t >::get(id);
	else
		return getParameterHandle(CastAny< std::wstring >::get(id));
}

int32_t input_IInputDevice_getCategory(IInputDevice* self)
{
	return (int32_t)self->getCategory();
}

std::vector< float > input_IInputDevice_getControlRange(IInputDevice* self, int32_t control)
{
	std::vector< float > range;
	float min, max;

	if (self->getControlRange(control, min, max))
	{
		range.resize(2);
		range[0] = min;
		range[1] = max;
	}

	return range;
}

int32_t input_IInputDevice_getDefaultControl(IInputDevice* self, int32_t controlType, bool analogue)
{
	int32_t control = -1;
	self->getDefaultControl((DefaultControl)controlType, analogue, control);
	return control;
}

int32_t input_InputSystem_getDeviceCount(InputSystem* self)
{
	return self->getDeviceCount();
}

IInputDevice* input_InputSystem_getDevice(InputSystem* self, int32_t index)
{
	return self->getDevice(index);
}

int32_t input_InputSystem_getDeviceCountByCategory(InputSystem* self, int32_t category, bool connected)
{
	return self->getDeviceCount((InputCategory)category, connected);
}

IInputDevice* input_InputSystem_getDeviceByCategory(InputSystem* self, int32_t category, int32_t index, bool connected)
{
	return self->getDevice((InputCategory)category, index, connected);
}

handle_t input_InputMapping_getHandle(const std::wstring& id)
{
	return getParameterHandle(id);
}

void input_InputMapping_reset_0(InputMapping* self)
{
	self->reset();
}

void input_InputMapping_reset_1(InputMapping* self, const Any& id)
{
	self->reset(anyHandle(id));
}

void input_InputMapping_setValue(InputMapping* self, const Any& id, float value)
{
	self->setValue(anyHandle(id), value);
}

float input_InputMapping_getValue(InputMapping* self, const Any& id)
{
	return self->getValue(anyHandle(id));
}

IInputSource* input_InputMapping_getSource(InputMapping* self, const Any& id)
{
	return self->getSource(anyHandle(id));
}

InputState* input_InputMapping_getState(InputMapping* self, const Any& id)
{
	return self->getState(anyHandle(id));
}

float input_InputMapping_getStateValue(InputMapping* self, const Any& id)
{
	return self->getStateValue(anyHandle(id));
}

float input_InputMapping_getStatePreviousValue(InputMapping* self, const Any& id)
{
	return self->getStatePreviousValue(anyHandle(id));
}

bool input_InputMapping_isStateDown(InputMapping* self, const Any& id)
{
	return self->isStateDown(anyHandle(id));
}

bool input_InputMapping_isStateUp(InputMapping* self, const Any& id)
{
	return self->isStateUp(anyHandle(id));
}

bool input_InputMapping_isStatePressed(InputMapping* self, const Any& id)
{
	return self->isStatePressed(anyHandle(id));
}

bool input_InputMapping_isStateReleased(InputMapping* self, const Any& id)
{
	return self->isStateReleased(anyHandle(id));
}

bool input_InputMapping_hasStateChanged(InputMapping* self, const Any& id)
{
	return self->hasStateChanged(anyHandle(id));
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputClassFactory", 0, InputClassFactory, IRuntimeClassFactory)

void InputClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classInputDriver = new AutoRuntimeClass< IInputDriver >();
	classInputDriver->addMethod("getDeviceCount", &IInputDriver::getDeviceCount);
	classInputDriver->addMethod("getDevice", &IInputDriver::getDevice);
	registrar->registerClass(classInputDriver);

	auto classInputDevice = new AutoRuntimeClass< IInputDevice >();

	for (int32_t i = 0; g_InputCategory_Keys[i].id != 0; ++i)
		classInputDevice->addConstant(wstombs(g_InputCategory_Keys[i].id), Any::fromInt32((int32_t)g_InputCategory_Keys[i].val));
	for (int32_t i = 0; g_DefaultControl_Keys[i].id != 0; ++i)
		classInputDevice->addConstant(wstombs(g_DefaultControl_Keys[i].id), Any::fromInt32((int32_t)g_DefaultControl_Keys[i].val));

	classInputDevice->addMethod("getName", &IInputDevice::getName);
	classInputDevice->addMethod("getCategory", &input_IInputDevice_getCategory);
	classInputDevice->addMethod("isConnected", &IInputDevice::isConnected);
	classInputDevice->addMethod("getControlCount", &IInputDevice::getControlCount);
	classInputDevice->addMethod("getControlName", &IInputDevice::getControlName);
	classInputDevice->addMethod("isControlAnalogue", &IInputDevice::isControlAnalogue);
	classInputDevice->addMethod("isControlStable", &IInputDevice::isControlStable);
	classInputDevice->addMethod("getControlValue", &IInputDevice::getControlValue);
	classInputDevice->addMethod("getControlRange", &input_IInputDevice_getControlRange);
	classInputDevice->addMethod("getDefaultControl", &input_IInputDevice_getDefaultControl);
	classInputDevice->addMethod("resetState", &IInputDevice::resetState);
	classInputDevice->addMethod("readState", &IInputDevice::readState);
	classInputDevice->addMethod("supportRumble", &IInputDevice::supportRumble);
	classInputDevice->addMethod("setExclusive", &IInputDevice::setExclusive);
	registrar->registerClass(classInputDevice);

	auto classInputSystem = new AutoRuntimeClass< InputSystem >();
	classInputSystem->addMethod("addDriver", &InputSystem::addDriver);
	classInputSystem->addMethod("removeDriver", &InputSystem::removeDriver);
	classInputSystem->addMethod("addDevice", &InputSystem::addDevice);
	classInputSystem->addMethod("removeDevice", &InputSystem::removeDevice);
	classInputSystem->addMethod("getDeviceCount", &input_InputSystem_getDeviceCount);
	classInputSystem->addMethod("getDevice", &input_InputSystem_getDevice);
	classInputSystem->addMethod("getDeviceCountByCategory", &input_InputSystem_getDeviceCountByCategory);
	classInputSystem->addMethod("getDeviceByCategory", &input_InputSystem_getDeviceByCategory);
	classInputSystem->addMethod("setExclusive", &InputSystem::setExclusive);
	classInputSystem->addMethod("update", &InputSystem::update);
	registrar->registerClass(classInputSystem);

	auto classInputMapping = new AutoRuntimeClass< InputMapping >();
	classInputMapping->addStaticMethod("getHandle", &input_InputMapping_getHandle);
	classInputMapping->addMethod("reset", &input_InputMapping_reset_0);
	classInputMapping->addMethod("reset", &input_InputMapping_reset_1);
	classInputMapping->addMethod("setValue", &input_InputMapping_setValue);
	classInputMapping->addMethod("getValue", &input_InputMapping_getValue);
	classInputMapping->addMethod("getSource", &input_InputMapping_getSource);
	classInputMapping->addMethod("getState", &input_InputMapping_getState);
	classInputMapping->addMethod("getStateValue", &input_InputMapping_getStateValue);
	classInputMapping->addMethod("getStatePreviousValue", &input_InputMapping_getStatePreviousValue);
	classInputMapping->addMethod("isStateDown", &input_InputMapping_isStateDown);
	classInputMapping->addMethod("isStateUp", &input_InputMapping_isStateUp);
	classInputMapping->addMethod("isStatePressed", &input_InputMapping_isStatePressed);
	classInputMapping->addMethod("isStateReleased", &input_InputMapping_isStateReleased);
	classInputMapping->addMethod("hasStateChanged", &input_InputMapping_hasStateChanged);
	classInputMapping->addMethod("getIdleDuration", &InputMapping::getIdleDuration);
	registrar->registerClass(classInputMapping);

	auto classInputSource = new AutoRuntimeClass< IInputSource >();
	classInputSource->addMethod("getDescription", &IInputSource::getDescription);
	registrar->registerClass(classInputSource);

	auto classInputState = new AutoRuntimeClass< InputState >();
	classInputState->addMethod("reset", &InputState::reset);
	classInputState->addMethod("getValue", &InputState::getValue);
	classInputState->addMethod("getPreviousValue", &InputState::getPreviousValue);
	classInputState->addMethod("isDown", &InputState::isDown);
	classInputState->addMethod("isUp", &InputState::isUp);
	classInputState->addMethod("isPressed", &InputState::isPressed);
	classInputState->addMethod("isReleased", &InputState::isReleased);
	classInputState->addMethod("hasChanged", &InputState::hasChanged);
	registrar->registerClass(classInputState);

	auto classRumbleEffect = new AutoRuntimeClass< RumbleEffect >();
	classRumbleEffect->addMethod("getDuration", &RumbleEffect::getDuration);
	registrar->registerClass(classRumbleEffect);

	auto classRumbleEffectPlayer = new AutoRuntimeClass< RumbleEffectPlayer >();
	classRumbleEffectPlayer->addMethod("play", &RumbleEffectPlayer::play);
	classRumbleEffectPlayer->addMethod("stop", &RumbleEffectPlayer::stop);
	classRumbleEffectPlayer->addMethod("stopAll", &RumbleEffectPlayer::stopAll);
	classRumbleEffectPlayer->addMethod("update", &RumbleEffectPlayer::update);
	registrar->registerClass(classRumbleEffectPlayer);
}

}

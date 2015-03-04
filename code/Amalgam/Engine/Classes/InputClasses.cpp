#include "Amalgam/Engine/Classes/InputClasses.h"
#include "Input/IInputDevice.h"
#include "Input/IInputDriver.h"
#include "Input/InputSystem.h"
#include "Input/Binding/IInputSource.h"
#include "Input/Binding/InputMapping.h"
#include "Input/Binding/InputState.h"
#include "Script/AutoScriptClass.h"
#include "Script/Boxes.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

input::handle_t anyHandle(const script::Any& id)
{
	if (script::CastAny< input::handle_t >::accept(id))
		return script::CastAny< input::handle_t >::get(id);
	else
		return input::getParameterHandle(script::CastAny< std::wstring >::get(id));
}

int32_t input_IInputDevice_getCategory(input::IInputDevice* self)
{
	return int32_t(self->getCategory());
}

std::vector< float > input_IInputDevice_getControlRange(input::IInputDevice* self, int32_t control)
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

int32_t input_IInputDevice_getDefaultControl(input::IInputDevice* self, int32_t controlType, bool analogue)
{
	int32_t control = -1;
	self->getDefaultControl((input::InputDefaultControlType)controlType, analogue, control);
	return control;
}

int32_t input_InputSystem_getDeviceCount(input::InputSystem* self)
{
	return self->getDeviceCount();
}

input::IInputDevice* input_InputSystem_getDevice(input::InputSystem* self, int32_t index)
{
	return self->getDevice(index);
}

int32_t input_InputSystem_getDeviceCountByCategory(input::InputSystem* self, int32_t category, bool connected)
{
	return self->getDeviceCount((input::InputCategory)category, connected);
}

input::IInputDevice* input_InputSystem_getDeviceByCategory(input::InputSystem* self, int32_t category, int32_t index, bool connected)
{
	return self->getDevice((input::InputCategory)category, index, connected);
}

input::handle_t input_InputMapping_getHandle(const std::wstring& id)
{
	return input::getParameterHandle(id);
}

void input_InputMapping_reset_0(input::InputMapping* self)
{
	self->reset();
}

void input_InputMapping_reset_1(input::InputMapping* self, const script::Any& id)
{
	self->reset(anyHandle(id));
}

void input_InputMapping_setValue(input::InputMapping* self, const script::Any& id, float value)
{
	self->setValue(anyHandle(id), value);
}

float input_InputMapping_getValue(input::InputMapping* self, const script::Any& id)
{
	return self->getValue(anyHandle(id));
}

input::IInputSource* input_InputMapping_getSource(input::InputMapping* self, const script::Any& id)
{
	return self->getSource(anyHandle(id));
}

input::InputState* input_InputMapping_getState(input::InputMapping* self, const script::Any& id)
{
	return self->getState(anyHandle(id));
}

float input_InputMapping_getStateValue(input::InputMapping* self, const script::Any& id)
{
	return self->getStateValue(anyHandle(id));
}

float input_InputMapping_getStatePreviousValue(input::InputMapping* self, const script::Any& id)
{
	return self->getStatePreviousValue(anyHandle(id));
}

bool input_InputMapping_isStateDown(input::InputMapping* self, const script::Any& id)
{
	return self->isStateDown(anyHandle(id));
}

bool input_InputMapping_isStateUp(input::InputMapping* self, const script::Any& id)
{
	return self->isStateUp(anyHandle(id));
}

bool input_InputMapping_isStatePressed(input::InputMapping* self, const script::Any& id)
{
	return self->isStatePressed(anyHandle(id));
}

bool input_InputMapping_isStateReleased(input::InputMapping* self, const script::Any& id)
{
	return self->isStateReleased(anyHandle(id));
}

bool input_InputMapping_hasStateChanged(input::InputMapping* self, const script::Any& id)
{
	return self->hasStateChanged(anyHandle(id));
}

		}

void registerInputClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< input::IInputDriver > > classInputDriver = new script::AutoScriptClass< input::IInputDriver >();
	classInputDriver->addMethod("getDeviceCount", &input::IInputDriver::getDeviceCount);
	classInputDriver->addMethod("getDevice", &input::IInputDriver::getDevice);
	scriptManager->registerClass(classInputDriver);

	Ref< script::AutoScriptClass< input::IInputDevice > > classInputDevice = new script::AutoScriptClass< input::IInputDevice >();
	classInputDevice->addMethod("getName", &input::IInputDevice::getName);
	classInputDevice->addMethod("getCategory", &input_IInputDevice_getCategory);
	classInputDevice->addMethod("isConnected", &input::IInputDevice::isConnected);
	classInputDevice->addMethod("getControlCount", &input::IInputDevice::getControlCount);
	classInputDevice->addMethod("getControlName", &input::IInputDevice::getControlName);
	classInputDevice->addMethod("isControlAnalogue", &input::IInputDevice::isControlAnalogue);
	classInputDevice->addMethod("isControlStable", &input::IInputDevice::isControlStable);
	classInputDevice->addMethod("getControlValue", &input::IInputDevice::getControlValue);
	classInputDevice->addMethod("getControlRange", &input_IInputDevice_getControlRange);
	classInputDevice->addMethod("getDefaultControl", &input_IInputDevice_getDefaultControl);
	classInputDevice->addMethod("resetState", &input::IInputDevice::resetState);
	classInputDevice->addMethod("readState", &input::IInputDevice::readState);
	classInputDevice->addMethod("supportRumble", &input::IInputDevice::supportRumble);
	classInputDevice->addMethod("setExclusive", &input::IInputDevice::setExclusive);
	scriptManager->registerClass(classInputDevice);

	Ref< script::AutoScriptClass< input::InputSystem > > classInputSystem = new script::AutoScriptClass< input::InputSystem >();
	classInputSystem->addMethod("addDriver", &input::InputSystem::addDriver);
	classInputSystem->addMethod("removeDriver", &input::InputSystem::removeDriver);
	classInputSystem->addMethod("addDevice", &input::InputSystem::addDevice);
	classInputSystem->addMethod("removeDevice", &input::InputSystem::removeDevice);
	classInputSystem->addMethod("getDeviceCount", &input_InputSystem_getDeviceCount);
	classInputSystem->addMethod("getDevice", &input_InputSystem_getDevice);
	classInputSystem->addMethod("getDeviceCountByCategory", &input_InputSystem_getDeviceCountByCategory);
	classInputSystem->addMethod("getDeviceByCategory", &input_InputSystem_getDeviceByCategory);
	classInputSystem->addMethod("setExclusive", &input::InputSystem::setExclusive);
	classInputSystem->addMethod("update", &input::InputSystem::update);
	scriptManager->registerClass(classInputSystem);

	Ref< script::AutoScriptClass< input::InputMapping > > classInputMapping = new script::AutoScriptClass< input::InputMapping >();
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
	classInputMapping->addMethod("getIdleDuration", &input::InputMapping::getIdleDuration);
	scriptManager->registerClass(classInputMapping);

	Ref< script::AutoScriptClass< input::IInputSource > > classInputSource = new script::AutoScriptClass< input::IInputSource >();
	classInputSource->addMethod("getDescription", &input::IInputSource::getDescription);
	scriptManager->registerClass(classInputSource);

	Ref< script::AutoScriptClass< input::InputState > > classInputState = new script::AutoScriptClass< input::InputState >();
	classInputState->addMethod("reset", &input::InputState::reset);
	classInputState->addMethod("getValue", &input::InputState::getValue);
	classInputState->addMethod("getPreviousValue", &input::InputState::getPreviousValue);
	classInputState->addMethod("isDown", &input::InputState::isDown);
	classInputState->addMethod("isUp", &input::InputState::isUp);
	classInputState->addMethod("isPressed", &input::InputState::isPressed);
	classInputState->addMethod("isReleased", &input::InputState::isReleased);
	classInputState->addMethod("hasChanged", &input::InputState::hasChanged);
	scriptManager->registerClass(classInputState);
}

	}
}

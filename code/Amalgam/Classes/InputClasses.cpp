#include "Amalgam/Classes/InputClasses.h"
#include "Input/IInputDevice.h"
#include "Input/IInputDriver.h"
#include "Input/InputSystem.h"
#include "Input/Binding/IInputSource.h"
#include "Input/Binding/InputMapping.h"
#include "Input/Binding/InputState.h"
#include "Script/AutoScriptClass.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

void input_InputMapping_reset_0(input::InputMapping* self)
{
	self->reset();
}

void input_InputMapping_reset_1(input::InputMapping* self, const std::wstring& id)
{
	self->reset(id);
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
	classInputDevice->addMethod("isConnected", &input::IInputDevice::isConnected);
	classInputDevice->addMethod("getControlCount", &input::IInputDevice::getControlCount);
	classInputDevice->addMethod("getControlName", &input::IInputDevice::getControlName);
	classInputDevice->addMethod("isControlAnalogue", &input::IInputDevice::isControlAnalogue);
	classInputDevice->addMethod("isControlStable", &input::IInputDevice::isControlStable);
	classInputDevice->addMethod("getControlValue", &input::IInputDevice::getControlValue);
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
	classInputSystem->addMethod("setExclusive", &input::InputSystem::setExclusive);
	classInputSystem->addMethod("update", &input::InputSystem::update);
	scriptManager->registerClass(classInputSystem);

	Ref< script::AutoScriptClass< input::InputMapping > > classInputMapping = new script::AutoScriptClass< input::InputMapping >();
	classInputMapping->addMethod("reset", &input_InputMapping_reset_0);
	classInputMapping->addMethod("reset", &input_InputMapping_reset_1);
	classInputMapping->addMethod("setValue", &input::InputMapping::setValue);
	classInputMapping->addMethod("getValue", &input::InputMapping::getValue);
	classInputMapping->addMethod("getSource", &input::InputMapping::getSource);
	classInputMapping->addMethod("getState", &input::InputMapping::getState);
	classInputMapping->addMethod("getStateValue", &input::InputMapping::getStateValue);
	classInputMapping->addMethod("getStatePreviousValue", &input::InputMapping::getStatePreviousValue);
	classInputMapping->addMethod("isStateDown", &input::InputMapping::isStateDown);
	classInputMapping->addMethod("isStateUp", &input::InputMapping::isStateUp);
	classInputMapping->addMethod("isStatePressed", &input::InputMapping::isStatePressed);
	classInputMapping->addMethod("isStateReleased", &input::InputMapping::isStateReleased);
	classInputMapping->addMethod("hasStateChanged", &input::InputMapping::hasStateChanged);
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

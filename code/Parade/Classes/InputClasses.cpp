#include "Input/IInputDevice.h"
#include "Input/IInputDriver.h"
#include "Input/InputSystem.h"
#include "Input/Binding/IInputSource.h"
#include "Input/Binding/InputMapping.h"
#include "Input/Binding/InputState.h"
#include "Parade/Classes/InputClasses.h"
#include "Script/AutoScriptClass.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace parade
	{

void registerInputClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< input::IInputDriver > > classInputDriver = new script::AutoScriptClass< input::IInputDriver >();
	classInputDriver->addMethod(L"getDeviceCount", &input::IInputDriver::getDeviceCount);
	classInputDriver->addMethod(L"getDevice", &input::IInputDriver::getDevice);
	scriptManager->registerClass(classInputDriver);

	Ref< script::AutoScriptClass< input::IInputDevice > > classInputDevice = new script::AutoScriptClass< input::IInputDevice >();
	classInputDevice->addMethod(L"getName", &input::IInputDevice::getName);
	classInputDevice->addMethod(L"isConnected", &input::IInputDevice::isConnected);
	classInputDevice->addMethod(L"getControlCount", &input::IInputDevice::getControlCount);
	classInputDevice->addMethod(L"getControlName", &input::IInputDevice::getControlName);
	classInputDevice->addMethod(L"isControlAnalogue", &input::IInputDevice::isControlAnalogue);
	classInputDevice->addMethod(L"isControlStable", &input::IInputDevice::isControlStable);
	classInputDevice->addMethod(L"getControlValue", &input::IInputDevice::getControlValue);
	classInputDevice->addMethod(L"resetState", &input::IInputDevice::resetState);
	classInputDevice->addMethod(L"readState", &input::IInputDevice::readState);
	classInputDevice->addMethod(L"supportRumble", &input::IInputDevice::supportRumble);
	classInputDevice->addMethod(L"setExclusive", &input::IInputDevice::setExclusive);
	scriptManager->registerClass(classInputDevice);

	Ref< script::AutoScriptClass< input::InputSystem > > classInputSystem = new script::AutoScriptClass< input::InputSystem >();
	classInputSystem->addMethod(L"addDriver", &input::InputSystem::addDriver);
	classInputSystem->addMethod(L"removeDriver", &input::InputSystem::removeDriver);
	classInputSystem->addMethod(L"addDevice", &input::InputSystem::addDevice);
	classInputSystem->addMethod(L"removeDevice", &input::InputSystem::removeDevice);
	classInputSystem->addMethod(L"setExclusive", &input::InputSystem::setExclusive);
	classInputSystem->addMethod(L"update", &input::InputSystem::update);
	scriptManager->registerClass(classInputSystem);

	Ref< script::AutoScriptClass< input::InputMapping > > classInputMapping = new script::AutoScriptClass< input::InputMapping >();
	classInputMapping->addMethod(L"reset", &input::InputMapping::reset);
	classInputMapping->addMethod(L"setValue", &input::InputMapping::setValue);
	classInputMapping->addMethod(L"getValue", &input::InputMapping::getValue);
	classInputMapping->addMethod(L"getSource", &input::InputMapping::getSource);
	classInputMapping->addMethod(L"getState", &input::InputMapping::getState);
	classInputMapping->addMethod(L"getStateValue", &input::InputMapping::getStateValue);
	classInputMapping->addMethod(L"getStatePreviousValue", &input::InputMapping::getStatePreviousValue);
	classInputMapping->addMethod(L"isStateDown", &input::InputMapping::isStateDown);
	classInputMapping->addMethod(L"isStateUp", &input::InputMapping::isStateUp);
	classInputMapping->addMethod(L"isStatePressed", &input::InputMapping::isStatePressed);
	classInputMapping->addMethod(L"isStateReleased", &input::InputMapping::isStateReleased);
	classInputMapping->addMethod(L"hasStateChanged", &input::InputMapping::hasStateChanged);
	scriptManager->registerClass(classInputMapping);

	Ref< script::AutoScriptClass< input::IInputSource > > classInputSource = new script::AutoScriptClass< input::IInputSource >();
	classInputSource->addMethod(L"getDescription", &input::IInputSource::getDescription);
	scriptManager->registerClass(classInputSource);

	Ref< script::AutoScriptClass< input::InputState > > classInputState = new script::AutoScriptClass< input::InputState >();
	classInputState->addMethod(L"reset", &input::InputState::reset);
	classInputState->addMethod(L"getValue", &input::InputState::getValue);
	classInputState->addMethod(L"getPreviousValue", &input::InputState::getPreviousValue);
	classInputState->addMethod(L"isDown", &input::InputState::isDown);
	classInputState->addMethod(L"isUp", &input::InputState::isUp);
	classInputState->addMethod(L"isPressed", &input::InputState::isPressed);
	classInputState->addMethod(L"isReleased", &input::InputState::isReleased);
	classInputState->addMethod(L"hasChanged", &input::InputState::hasChanged);
	scriptManager->registerClass(classInputState);
}

	}
}

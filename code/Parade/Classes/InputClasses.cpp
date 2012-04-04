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
	Ref< script::AutoScriptClass< input::InputSystem > > classInputSystem = new script::AutoScriptClass< input::InputSystem >();
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

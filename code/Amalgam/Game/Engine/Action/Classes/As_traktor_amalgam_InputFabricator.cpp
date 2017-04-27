/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Amalgam/Game/Engine/Action/AsInputFabricator.h"
#include "Amalgam/Game/Engine/Action/Classes/As_traktor_amalgam_InputFabricator.h"
#include "Flash/Action/ActionContext.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.As_traktor_amalgam_InputFabricator", As_traktor_amalgam_InputFabricator, flash::ActionClass)

As_traktor_amalgam_InputFabricator::As_traktor_amalgam_InputFabricator(flash::ActionContext* context, IEnvironment* environment)
:	flash::ActionClass(context, "traktor.amalgam.InputFabricator")
,	m_environment(environment)
{
	setMember("KEYBOARD", flash::ActionValue((int32_t)input::CtKeyboard));
	setMember("MOUSE", flash::ActionValue((int32_t)input::CtMouse));
	setMember("JOYSTICK", flash::ActionValue((int32_t)input::CtJoystick));
	setMember("WHEEL", flash::ActionValue((int32_t)input::CtWheel));

	Ref< flash::ActionObject > prototype = new flash::ActionObject(context);

	prototype->setMember("getSourceDescription", flash::ActionValue(flash::createNativeFunction(context, &AsInputFabricator::getSourceDescription)));
	prototype->setMember("fabricateSource", flash::ActionValue(flash::createNativeFunction(context, this, &As_traktor_amalgam_InputFabricator::InputFabricator_fabricateSource)));
	prototype->setMember("isFabricating", flash::ActionValue(flash::createNativeFunction(context, &AsInputFabricator::isFabricating)));
	prototype->setMember("abortedFabricating", flash::ActionValue(flash::createNativeFunction(context, &AsInputFabricator::abortedFabricating)));
	prototype->setMember("resetInputSource", flash::ActionValue(flash::createNativeFunction(context, &AsInputFabricator::resetInputSource)));
	prototype->setMember("apply", flash::ActionValue(flash::createNativeFunction(context, &AsInputFabricator::apply)));
	prototype->setMember("revert", flash::ActionValue(flash::createNativeFunction(context, &AsInputFabricator::revert)));

	prototype->setMember("constructor", flash::ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", flash::ActionValue(prototype));
}

void As_traktor_amalgam_InputFabricator::initialize(flash::ActionObject* self)
{
}

void As_traktor_amalgam_InputFabricator::construct(flash::ActionObject* self, const flash::ActionValueArray& args)
{
	Ref< AsInputFabricator > inp = new AsInputFabricator(m_environment);
	self->setRelay(inp);
}

flash::ActionValue As_traktor_amalgam_InputFabricator::xplicit(const flash::ActionValueArray& args)
{
	return flash::ActionValue();
}

void As_traktor_amalgam_InputFabricator::InputFabricator_fabricateSource(flash::CallArgs& ca)
{
	Ref< AsInputFabricator > inp = ca.self->getRelay< AsInputFabricator >();

	bool r = inp->fabricateSource(
		ca.args[0].getWideString(),
		ca.args[1].getInteger(),
		ca.args[2].getBoolean()
	);

	ca.ret = flash::ActionValue(r);
}

	}
}

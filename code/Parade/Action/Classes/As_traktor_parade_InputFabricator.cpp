#include "Flash/Action/ActionContext.h"
#include "Parade/Action/AsInputFabricator.h"
#include "Parade/Action/Classes/As_traktor_parade_InputFabricator.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.As_traktor_parade_InputFabricator", As_traktor_parade_InputFabricator, flash::ActionClass)

As_traktor_parade_InputFabricator::As_traktor_parade_InputFabricator(flash::ActionContext* context, amalgam::IEnvironment* environment)
:	flash::ActionClass(context, "traktor.parade.InputFabricator")
,	m_environment(environment)
{
	setMember("KEYBOARD", flash::ActionValue((flash::avm_number_t)input::CtKeyboard));
	setMember("MOUSE", flash::ActionValue((flash::avm_number_t)input::CtMouse));
	setMember("JOYSTICK", flash::ActionValue((flash::avm_number_t)input::CtJoystick));
	setMember("WHEEL", flash::ActionValue((flash::avm_number_t)input::CtWheel));

	Ref< flash::ActionObject > prototype = new flash::ActionObject(context);

	prototype->setMember("getSourceDescription", flash::ActionValue(flash::createNativeFunction(context, &AsInputFabricator::getSourceDescription)));
	prototype->setMember("fabricateSource", flash::ActionValue(flash::createNativeFunction(context, this, &As_traktor_parade_InputFabricator::InputFabricator_fabricateSource)));
	prototype->setMember("isFabricating", flash::ActionValue(flash::createNativeFunction(context, &AsInputFabricator::isFabricating)));
	prototype->setMember("abortedFabricating", flash::ActionValue(flash::createNativeFunction(context, &AsInputFabricator::abortedFabricating)));
	prototype->setMember("resetInputSource", flash::ActionValue(flash::createNativeFunction(context, &AsInputFabricator::resetInputSource)));

	prototype->setMember("constructor", flash::ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", flash::ActionValue(prototype));
}

void As_traktor_parade_InputFabricator::initialize(flash::ActionObject* self)
{
}

void As_traktor_parade_InputFabricator::construct(flash::ActionObject* self, const flash::ActionValueArray& args)
{
	flash::ActionContext* context = getContext();
	T_ASSERT (context);

	Ref< AsInputFabricator > inp = new AsInputFabricator(m_environment);

	self->setRelay(inp);
}

flash::ActionValue As_traktor_parade_InputFabricator::xplicit(const flash::ActionValueArray& args)
{
	return flash::ActionValue();
}

void As_traktor_parade_InputFabricator::InputFabricator_fabricateSource(flash::CallArgs& ca)
{
	Ref< AsInputFabricator > inp = ca.self->getRelay< AsInputFabricator >();

	bool r = inp->fabricateSource(
		ca.args[0].getWideString(),
		int32_t(ca.args[1].getNumber()),
		ca.args[2].getBoolean()
	);

	ca.ret = flash::ActionValue(r);
}

	}
}

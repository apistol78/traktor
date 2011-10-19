#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Classes/Tween.h"
#include "Flash/Action/Avm1/Classes/As_mx_transitions_Tween.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_mx_transitions_Tween", As_mx_transitions_Tween, ActionClass)

As_mx_transitions_Tween::As_mx_transitions_Tween()
:	ActionClass("mx.transitions.Tween")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember("continueTo", ActionValue(createNativeFunction(&Tween::continueTo)));
	prototype->setMember("fforward", ActionValue(createNativeFunction(&Tween::fforward)));
	prototype->setMember("nextFrame", ActionValue(createNativeFunction(&Tween::nextFrame)));
	prototype->setMember("prevFrame", ActionValue(createNativeFunction(&Tween::prevFrame)));
	prototype->setMember("resume", ActionValue(createNativeFunction(&Tween::resume)));
	prototype->setMember("rewind", ActionValue(createNativeFunction(&Tween::rewind)));
	prototype->setMember("start", ActionValue(createNativeFunction(&Tween::start)));
	prototype->setMember("stop", ActionValue(createNativeFunction(&Tween::stop)));
	prototype->setMember("yoyo", ActionValue(createNativeFunction(&Tween::yoyo)));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

Ref< ActionObject > As_mx_transitions_Tween::alloc(ActionContext* context)
{
	return new Tween();
}

void As_mx_transitions_Tween::init(ActionContext* context, ActionObject* self, const ActionValueArray& args)
{
	if (args.size() < 7)
		return;

	checked_type_cast< Tween* >(self)->init(
		context,
		args[0].getObject(),
		args[1].getString(),
		args[2].getObject< ActionFunction >(),
		args[3].getNumber(),
		args[4].getNumber(),
		args[5].getNumber(),
		args[6].getBoolean()
	);
}

	}
}

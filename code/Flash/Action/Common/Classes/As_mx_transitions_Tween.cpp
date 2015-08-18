#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Tween.h"
#include "Flash/Action/Common/Classes/As_mx_transitions_Tween.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_mx_transitions_Tween", As_mx_transitions_Tween, ActionClass)

As_mx_transitions_Tween::As_mx_transitions_Tween(ActionContext* context)
:	ActionClass(context, "mx.transitions.Tween")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("continueTo", ActionValue(createNativeFunction(context, &Tween::continueTo)));
	prototype->setMember("fforward", ActionValue(createNativeFunction(context, &Tween::fforward)));
	prototype->setMember("nextFrame", ActionValue(createNativeFunction(context, &Tween::nextFrame)));
	prototype->setMember("prevFrame", ActionValue(createNativeFunction(context, &Tween::prevFrame)));
	prototype->setMember("resume", ActionValue(createNativeFunction(context, &Tween::resume)));
	prototype->setMember("rewind", ActionValue(createNativeFunction(context, &Tween::rewind)));
	prototype->setMember("start", ActionValue(createNativeFunction(context, &Tween::start)));
	prototype->setMember("stop", ActionValue(createNativeFunction(context, &Tween::stop)));
	prototype->setMember("yoyo", ActionValue(createNativeFunction(context, &Tween::yoyo)));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void As_mx_transitions_Tween::initialize(ActionObject* self)
{
}

void As_mx_transitions_Tween::construct(ActionObject* self, const ActionValueArray& args)
{
	Ref< Tween > tw = new Tween(getContext());
	self->setRelay(tw);

	if (args.size() >= 7)
	{
		tw->init(
			args[0].getObjectAlways(getContext()),
			args[1].getString(),
			args[2].getObject< ActionFunction >(),
			args[3].getNumber(),
			args[4].getNumber(),
			args[5].getNumber(),
			args[6].getBoolean()
		);
	}
}

ActionValue As_mx_transitions_Tween::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

	}
}

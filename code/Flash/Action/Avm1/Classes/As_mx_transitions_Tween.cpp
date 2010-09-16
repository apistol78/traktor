#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Classes/Tween.h"
#include "Flash/Action/Avm1/Classes/As_mx_transitions_Tween.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_mx_transitions_Tween", As_mx_transitions_Tween, ActionClass)

As_mx_transitions_Tween::As_mx_transitions_Tween()
:	ActionClass(L"mx.transitions.Tween")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"continueTo", ActionValue(createNativeFunction(this, &As_mx_transitions_Tween::Tween_continueTo)));
	prototype->setMember(L"fforward", ActionValue(createNativeFunction(this, &As_mx_transitions_Tween::Tween_fforward)));
	prototype->setMember(L"nextFrame", ActionValue(createNativeFunction(this, &As_mx_transitions_Tween::Tween_nextFrame)));
	prototype->setMember(L"prevFrame", ActionValue(createNativeFunction(this, &As_mx_transitions_Tween::Tween_prevFrame)));
	prototype->setMember(L"resume", ActionValue(createNativeFunction(this, &As_mx_transitions_Tween::Tween_resume)));
	prototype->setMember(L"rewind", ActionValue(createNativeFunction(this, &As_mx_transitions_Tween::Tween_rewind)));
	prototype->setMember(L"start", ActionValue(createNativeFunction(this, &As_mx_transitions_Tween::Tween_start)));
	prototype->setMember(L"stop", ActionValue(createNativeFunction(this, &As_mx_transitions_Tween::Tween_stop)));
	prototype->setMember(L"yoyo", ActionValue(createNativeFunction(this, &As_mx_transitions_Tween::Tween_yoyo)));
	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
}

ActionValue As_mx_transitions_Tween::construct(ActionContext* context, const ActionValueArray& args)
{
	if (args.size() < 7)
		return ActionValue();

	Ref< Tween > tween = new Tween(
		context,
		args[0].getObjectSafe(),
		args[1].getStringSafe(),
		args[2].getObjectSafe< ActionFunction >(),
		args[3].getNumberSafe(),
		args[4].getNumberSafe(),
		args[5].getNumberSafe(),
		args[6].getBooleanSafe()
	);

	return ActionValue(tween);
}

void As_mx_transitions_Tween::Tween_continueTo(CallArgs& ca)
{
	Tween* tween = checked_type_cast< Tween*, false >(ca.self);
	tween->continueTo(
		ca.args[0].getNumberSafe(),
		ca.args[1].getNumberSafe()
	);
}

void As_mx_transitions_Tween::Tween_fforward(CallArgs& ca)
{
	Tween* tween = checked_type_cast< Tween*, false >(ca.self);
	tween->fforward();
}

void As_mx_transitions_Tween::Tween_nextFrame(CallArgs& ca)
{
	Tween* tween = checked_type_cast< Tween*, false >(ca.self);
	tween->nextFrame();
}

void As_mx_transitions_Tween::Tween_prevFrame(CallArgs& ca)
{
	Tween* tween = checked_type_cast< Tween*, false >(ca.self);
	tween->prevFrame();
}

void As_mx_transitions_Tween::Tween_resume(CallArgs& ca)
{
	Tween* tween = checked_type_cast< Tween*, false >(ca.self);
	tween->resume();
}

void As_mx_transitions_Tween::Tween_rewind(CallArgs& ca)
{
	Tween* tween = checked_type_cast< Tween*, false >(ca.self);
	tween->rewind(ca.args[0].getNumberSafe());
}

void As_mx_transitions_Tween::Tween_start(CallArgs& ca)
{
	Tween* tween = checked_type_cast< Tween*, false >(ca.self);
	tween->start();
}

void As_mx_transitions_Tween::Tween_stop(CallArgs& ca)
{
	Tween* tween = checked_type_cast< Tween*, false >(ca.self);
	tween->stop();
}

void As_mx_transitions_Tween::Tween_yoyo(CallArgs& ca)
{
	Tween* tween = checked_type_cast< Tween*, false >(ca.self);
	tween->yoyo();
}

	}
}

#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Classes/Tween.h"
#include "Flash/Action/Avm1/Classes/AsTween.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsTween", AsTween, ActionClass)

Ref< AsTween > AsTween::getInstance()
{
	static Ref< AsTween > instance = 0;
	if (!instance)
	{
		instance = new AsTween();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

AsTween::AsTween()
:	ActionClass(L"Tween")
{
}

void AsTween::createPrototype()
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"__proto__", ActionValue(AsObject::getInstance()));
	prototype->setMember(L"continueTo", ActionValue(createNativeFunction(this, &AsTween::Tween_continueTo)));
	prototype->setMember(L"fforward", ActionValue(createNativeFunction(this, &AsTween::Tween_fforward)));
	prototype->setMember(L"nextFrame", ActionValue(createNativeFunction(this, &AsTween::Tween_nextFrame)));
	prototype->setMember(L"prevFrame", ActionValue(createNativeFunction(this, &AsTween::Tween_prevFrame)));
	prototype->setMember(L"resume", ActionValue(createNativeFunction(this, &AsTween::Tween_resume)));
	prototype->setMember(L"rewind", ActionValue(createNativeFunction(this, &AsTween::Tween_rewind)));
	prototype->setMember(L"start", ActionValue(createNativeFunction(this, &AsTween::Tween_start)));
	prototype->setMember(L"stop", ActionValue(createNativeFunction(this, &AsTween::Tween_stop)));
	prototype->setMember(L"yoyo", ActionValue(createNativeFunction(this, &AsTween::Tween_yoyo)));
	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
}

ActionValue AsTween::construct(ActionContext* context, const ActionValueArray& args)
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

void AsTween::Tween_continueTo(CallArgs& ca)
{
	Tween* tween = checked_type_cast< Tween*, false >(ca.self);
	tween->continueTo(
		ca.args[0].getNumberSafe(),
		ca.args[1].getNumberSafe()
	);
}

void AsTween::Tween_fforward(CallArgs& ca)
{
	Tween* tween = checked_type_cast< Tween*, false >(ca.self);
	tween->fforward();
}

void AsTween::Tween_nextFrame(CallArgs& ca)
{
	Tween* tween = checked_type_cast< Tween*, false >(ca.self);
	tween->nextFrame();
}

void AsTween::Tween_prevFrame(CallArgs& ca)
{
	Tween* tween = checked_type_cast< Tween*, false >(ca.self);
	tween->prevFrame();
}

void AsTween::Tween_resume(CallArgs& ca)
{
	Tween* tween = checked_type_cast< Tween*, false >(ca.self);
	tween->resume();
}

void AsTween::Tween_rewind(CallArgs& ca)
{
	Tween* tween = checked_type_cast< Tween*, false >(ca.self);
	tween->rewind(ca.args[0].getNumberSafe());
}

void AsTween::Tween_start(CallArgs& ca)
{
	Tween* tween = checked_type_cast< Tween*, false >(ca.self);
	tween->start();
}

void AsTween::Tween_stop(CallArgs& ca)
{
	Tween* tween = checked_type_cast< Tween*, false >(ca.self);
	tween->stop();
}

void AsTween::Tween_yoyo(CallArgs& ca)
{
	Tween* tween = checked_type_cast< Tween*, false >(ca.self);
	tween->yoyo();
}

	}
}

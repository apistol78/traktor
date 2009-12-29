#include "Flash/Action/ActionContext.h"
#include "Flash/Action/Avm1/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsSystem.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"
#include "Flash/Action/Avm1/Classes/AsSecurity.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsSystem", AsSystem, ActionClass)

Ref< AsSystem > AsSystem::getInstance()
{
	static Ref< AsSystem > instance = 0;
	if (!instance)
	{
		instance = new AsSystem();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

AsSystem::AsSystem()
:	ActionClass(L"System")
{
}

void AsSystem::createPrototype()
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"security", ActionValue::fromObject(AsSecurity::getInstance()));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue::fromObject(prototype));
}

ActionValue AsSystem::construct(ActionContext* context, const args_t& args)
{
	return ActionValue();
}

	}
}

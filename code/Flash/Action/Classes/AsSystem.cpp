#include "Flash/Action/Classes/AsSystem.h"
#include "Flash/Action/Classes/AsObject.h"
#include "Flash/Action/Classes/AsSecurity.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/ActionContext.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsSystem", AsSystem, ActionClass)

AsSystem* AsSystem::getInstance()
{
	static AsSystem* instance = 0;
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
	Ref< ActionObject > prototype = gc_new< ActionObject >();

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

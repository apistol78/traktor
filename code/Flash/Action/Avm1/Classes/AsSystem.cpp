#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsSystem.h"
#include "Flash/Action/Avm1/Classes/AsSecurity.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsSystem", AsSystem, ActionClass)

AsSystem::AsSystem(ActionContext* context)
:	ActionClass(context, "System")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("security", ActionValue(new AsSecurity(context)));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void AsSystem::init(ActionObject* self, const ActionValueArray& args)
{
}

void AsSystem::coerce(ActionObject* self) const
{
	T_FATAL_ERROR;
}

	}
}

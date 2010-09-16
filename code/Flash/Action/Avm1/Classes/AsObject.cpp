#include "Core/Platform.h"
#include "Core/Log/Log.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsObject", AsObject, ActionClass)

AsObject::AsObject()
:	ActionClass(L"Object")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"addProperty", ActionValue(createNativeFunction(this, &AsObject::Object_addProperty)));
	prototype->setMember(L"hasOwnProperty", ActionValue(createNativeFunction(this, &AsObject::Object_hasOwnProperty)));
	prototype->setMember(L"isPropertyEnumerable", ActionValue(createNativeFunction(this, &AsObject::Object_isPropertyEnumerable)));
	prototype->setMember(L"isPrototypeOf", ActionValue(createNativeFunction(this, &AsObject::Object_isPrototypeOf)));
	prototype->setMember(L"registerClass", ActionValue(createNativeFunction(this, &AsObject::Object_registerClass)));
	prototype->setMember(L"toString", ActionValue(createNativeFunction(this, &AsObject::Object_toString)));
	prototype->setMember(L"unwatch", ActionValue(createNativeFunction(this, &AsObject::Object_unwatch)));
	prototype->setMember(L"valueOf", ActionValue(createNativeFunction(this, &AsObject::Object_valueOf)));
	prototype->setMember(L"watch", ActionValue(createNativeFunction(this, &AsObject::Object_watch)));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
}

ActionValue AsObject::construct(ActionContext* context, const ActionValueArray& args)
{
	return ActionValue(new ActionObject(L"Object"));
}

void AsObject::Object_addProperty(CallArgs& ca)
{
	T_ASSERT (ca.self);
	T_ASSERT (ca.args.size() >= 3);

	std::wstring propertyName = ca.args[0].getStringSafe();
	Ref< ActionFunction > propertyGet = checked_type_cast< ActionFunction* >(ca.args[1].getObject());
	Ref< ActionFunction > propertySet = checked_type_cast< ActionFunction* >(ca.args[2].getObject());

	ca.self->addProperty(propertyName, propertyGet, propertySet);
}

void AsObject::Object_hasOwnProperty(CallArgs& ca)
{
	T_ASSERT (ca.self);
	T_ASSERT (ca.args.size() >= 1);

	std::wstring propertyName = ca.args[0].getStringSafe();
	ca.ret = ActionValue(ca.self->hasOwnProperty(propertyName));
}

void AsObject::Object_isPropertyEnumerable(CallArgs& ca)
{
	ca.ret = ActionValue(true);
}

void AsObject::Object_isPrototypeOf(CallArgs& ca)
{
	ca.ret = ActionValue(false);
}

void AsObject::Object_registerClass(CallArgs& ca)
{
	Ref< ActionObject > global = ca.context->getGlobal();
	std::wstring movieClipName = ca.args[0].getStringSafe();
	
	if (ca.args.size() >= 2)
	{
		ActionValue theClass = ca.args[1];
		if (theClass.isObject())
		{
			global->setMember(movieClipName, theClass);
			ca.ret = ActionValue(true);
		}
		else
			ca.ret = ActionValue(false);
	}
	else
		ca.ret = ActionValue(global->deleteMember(movieClipName));
}

void AsObject::Object_toString(CallArgs& ca)
{
	T_ASSERT (ca.self);
	ca.ret = ActionValue(ca.self->toString());
}

void AsObject::Object_unwatch(CallArgs& ca)
{
}

void AsObject::Object_valueOf(CallArgs& ca)
{
	T_ASSERT (ca.self);
	ca.ret = ActionValue(ca.self->valueOf());
}

void AsObject::Object_watch(CallArgs& ca)
{
}

	}
}

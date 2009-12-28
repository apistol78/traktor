#include "Flash/Action/Avm1/Classes/AsTextField.h"
#include "Flash/Action/Avm1/Classes/AsTextField_StyleSheet.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"
#include "Flash/Action/Avm1/ActionFunctionNative.h"
#include "Flash/Action/Avm1/ActionContext.h"
#include "Flash/FlashEditInstance.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsTextField", AsTextField, ActionClass)

Ref< AsTextField > AsTextField::getInstance()
{
	static Ref< AsTextField > instance = 0;
	if (!instance)
	{
		instance = new AsTextField();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

AsTextField::AsTextField()
:	ActionClass(L"TextField")
{
}

void AsTextField::createPrototype()
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"__proto__", ActionValue::fromObject(AsObject::getInstance()));
	prototype->setMember(L"StyleSheet", ActionValue::fromObject(AsTextField_StyleSheet::getInstance()));

	prototype->addProperty(L"text", createNativeFunction(this, &AsTextField::TextField_get_text), createNativeFunction(this, &AsTextField::TextField_set_text));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue::fromObject(prototype));
}

ActionValue AsTextField::construct(ActionContext* context, const args_t& args)
{
	return ActionValue();
}

void AsTextField::TextField_get_text(CallArgs& ca)
{
	//Ref< FlashEditInstance > editInstance = dynamic_type_cast< FlashEditInstance* >(ca.self);
	//if (editInstance)
	//	ca.ret = editInstance->getText();
	//else
	//	log::warning << L"Cannot get text from static text fields" << Endl;
}

void AsTextField::TextField_set_text(CallArgs& ca)
{
	Ref< FlashEditInstance > editInstance = dynamic_type_cast< FlashEditInstance* >(ca.self);
	if (editInstance)
		editInstance->parseText(ca.args[0].getStringSafe());
	else
		log::warning << L"Cannot set text on static text fields" << Endl;
}

	}
}

#include "Flash/Action/Classes/AsTextField_StyleSheet.h"
#include "Flash/Action/Classes/AsObject.h"
#include "Flash/Action/ActionDate.h"
#include "Flash/Action/ActionFunctionNative.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsTextField_StyleSheet", AsTextField_StyleSheet, ActionClass)

Ref< AsTextField_StyleSheet > AsTextField_StyleSheet::getInstance()
{
	static AsTextField_StyleSheet* instance = 0;
	if (!instance)
	{
		instance = new AsTextField_StyleSheet();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

AsTextField_StyleSheet::AsTextField_StyleSheet()
:	ActionClass(L"TextField.StyleSheet")
{
}

void AsTextField_StyleSheet::createPrototype()
{
	Ref< ActionObject > prototype = gc_new< ActionObject >();

	prototype->setMember(L"__proto__", ActionValue::fromObject(AsObject::getInstance()));
	prototype->setMember(L"clear", createNativeFunctionValue(this, &AsTextField_StyleSheet::TextField_StyleSheet_clear));
	prototype->setMember(L"getStyle", createNativeFunctionValue(this, &AsTextField_StyleSheet::TextField_StyleSheet_getStyle));
	prototype->setMember(L"getStyleNames", createNativeFunctionValue(this, &AsTextField_StyleSheet::TextField_StyleSheet_getStyleNames));
	prototype->setMember(L"load", createNativeFunctionValue(this, &AsTextField_StyleSheet::TextField_StyleSheet_load));
	prototype->setMember(L"parseCSS", createNativeFunctionValue(this, &AsTextField_StyleSheet::TextField_StyleSheet_parseCSS));
	prototype->setMember(L"setStyle", createNativeFunctionValue(this, &AsTextField_StyleSheet::TextField_StyleSheet_setStyle));
	prototype->setMember(L"transform", createNativeFunctionValue(this, &AsTextField_StyleSheet::TextField_StyleSheet_transform));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue::fromObject(prototype));
}

ActionValue AsTextField_StyleSheet::construct(ActionContext* context, const args_t& args)
{
	return ActionValue::fromObject(gc_new< ActionObject >(this));
}

void AsTextField_StyleSheet::TextField_StyleSheet_clear(CallArgs& ca)
{
}

void AsTextField_StyleSheet::TextField_StyleSheet_getStyle(CallArgs& ca)
{
}

void AsTextField_StyleSheet::TextField_StyleSheet_getStyleNames(CallArgs& ca)
{
}

void AsTextField_StyleSheet::TextField_StyleSheet_load(CallArgs& ca)
{
}

void AsTextField_StyleSheet::TextField_StyleSheet_parseCSS(CallArgs& ca)
{
}

void AsTextField_StyleSheet::TextField_StyleSheet_setStyle(CallArgs& ca)
{
}

void AsTextField_StyleSheet::TextField_StyleSheet_transform(CallArgs& ca)
{
}

	}
}

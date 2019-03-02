#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Classes/AsTextField_StyleSheet.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsTextField_StyleSheet", AsTextField_StyleSheet, ActionClass)

AsTextField_StyleSheet::AsTextField_StyleSheet(ActionContext* context)
:	ActionClass(context, "TextField.StyleSheet")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("clear", ActionValue(createNativeFunction(context, this, &AsTextField_StyleSheet::TextField_StyleSheet_clear)));
	prototype->setMember("getStyle", ActionValue(createNativeFunction(context, this, &AsTextField_StyleSheet::TextField_StyleSheet_getStyle)));
	prototype->setMember("getStyleNames", ActionValue(createNativeFunction(context, this, &AsTextField_StyleSheet::TextField_StyleSheet_getStyleNames)));
	prototype->setMember("load", ActionValue(createNativeFunction(context, this, &AsTextField_StyleSheet::TextField_StyleSheet_load)));
	prototype->setMember("parseCSS", ActionValue(createNativeFunction(context, this, &AsTextField_StyleSheet::TextField_StyleSheet_parseCSS)));
	prototype->setMember("setStyle", ActionValue(createNativeFunction(context, this, &AsTextField_StyleSheet::TextField_StyleSheet_setStyle)));
	prototype->setMember("transform", ActionValue(createNativeFunction(context, this, &AsTextField_StyleSheet::TextField_StyleSheet_transform)));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void AsTextField_StyleSheet::initialize(ActionObject* self)
{
}

void AsTextField_StyleSheet::construct(ActionObject* self, const ActionValueArray& args)
{
}

ActionValue AsTextField_StyleSheet::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

void AsTextField_StyleSheet::TextField_StyleSheet_clear(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"TextField::StyleSheet::clear not implemented" << Endl;
	)
}

void AsTextField_StyleSheet::TextField_StyleSheet_getStyle(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"TextField::StyleSheet::getStyle not implemented" << Endl;
	)
}

void AsTextField_StyleSheet::TextField_StyleSheet_getStyleNames(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"TextField::StyleSheet::getStyleNames not implemented" << Endl;
	)
}

void AsTextField_StyleSheet::TextField_StyleSheet_load(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"TextField::StyleSheet::load not implemented" << Endl;
	)
}

void AsTextField_StyleSheet::TextField_StyleSheet_parseCSS(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"TextField::StyleSheet::parseCSS not implemented" << Endl;
	)
}

void AsTextField_StyleSheet::TextField_StyleSheet_setStyle(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"TextField::StyleSheet::setStyle not implemented" << Endl;
	)
}

void AsTextField_StyleSheet::TextField_StyleSheet_transform(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"TextField::StyleSheet::transform not implemented" << Endl;
	)
}

	}
}

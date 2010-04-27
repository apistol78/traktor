#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Flash/FlashEditInstance.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsTextField.h"
#include "Flash/Action/Avm1/Classes/AsTextField_StyleSheet.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"

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

	prototype->setMember(L"__proto__", ActionValue(AsObject::getInstance()));
	prototype->setMember(L"StyleSheet", ActionValue(AsTextField_StyleSheet::getInstance()));

	prototype->addProperty(L"text", createNativeFunction(this, &AsTextField::TextField_get_text), createNativeFunction(this, &AsTextField::TextField_set_text));
	prototype->addProperty(L"textWidth", createNativeFunction(this, &AsTextField::TextField_get_textWidth), 0);
	prototype->addProperty(L"textHeight", createNativeFunction(this, &AsTextField::TextField_get_textHeight), 0);

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
}

ActionValue AsTextField::construct(ActionContext* context, const ActionValueArray& args)
{
	return ActionValue();
}

std::wstring AsTextField::TextField_get_text(FlashEditInstance* editInstance) const
{
	StringOutputStream ss;

	const FlashEditInstance::text_t& text = editInstance->getText();
	for (FlashEditInstance::text_t::const_iterator i = text.begin(); i != text.end(); ++i)
		ss << *i << Endl;

	return ss.str();
}

void AsTextField::TextField_set_text(FlashEditInstance* editInstance, const std::wstring& text) const
{
	editInstance->parseText(text);
}

avm_number_t AsTextField::TextField_get_textWidth(FlashEditInstance* editInstance) const
{
	return editInstance->getTextWidth();
}

avm_number_t AsTextField::TextField_get_textHeight(FlashEditInstance* editInstance) const
{
	return editInstance->getTextHeight();
}

	}
}

#include "I18N/Format.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsI18N.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsI18N", AsI18N, ActionClass)

Ref< AsI18N > AsI18N::getInstance()
{
	static Ref< AsI18N > instance = 0;
	if (!instance)
	{
		instance = new AsI18N();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

AsI18N::AsI18N()
:	ActionClass(L"I18N")
{
}

void AsI18N::createPrototype()
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"__proto__", ActionValue(AsObject::getInstance()));
	prototype->setMember(L"format", createNativeFunctionValue(this, &AsI18N::I18N_format));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
}

ActionValue AsI18N::construct(ActionContext* context, const args_t& args)
{
	return ActionValue();
}

void AsI18N::I18N_format(CallArgs& ca)
{
	std::wstring text;

	switch (ca.args.size())
	{
	case 1:
		text = i18n::Format(
			ca.args[0].getStringSafe()
		);
		break;

	case 2:
		text = i18n::Format(
			ca.args[0].getStringSafe(),
			ca.args[1].getStringSafe()
		);
		break;

	case 3:
		text = i18n::Format(
			ca.args[0].getStringSafe(),
			ca.args[1].getStringSafe(),
			ca.args[2].getStringSafe()
		);
		break;

	case 4:
		text = i18n::Format(
			ca.args[0].getStringSafe(),
			ca.args[1].getStringSafe(),
			ca.args[2].getStringSafe(),
			ca.args[3].getStringSafe()
		);
		break;

	case 5:
		text = i18n::Format(
			ca.args[0].getStringSafe(),
			ca.args[1].getStringSafe(),
			ca.args[2].getStringSafe(),
			ca.args[3].getStringSafe(),
			ca.args[4].getStringSafe()
		);
		break;
	}

	ca.ret = ActionValue(text);
}

	}
}

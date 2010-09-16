#include "I18N/Format.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsI18N.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsI18N", AsI18N, ActionClass)

AsI18N::AsI18N()
:	ActionClass(L"I18N")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"format", ActionValue(createNativeFunction(this, &AsI18N::I18N_format)));
	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
}

ActionValue AsI18N::construct(ActionContext* context, const ActionValueArray& args)
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

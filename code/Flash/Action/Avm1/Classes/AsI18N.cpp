#include "I18N/Format.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsI18N.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsI18N", AsI18N, ActionClass)

AsI18N::AsI18N()
:	ActionClass("I18N")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember("format", ActionValue(createNativeFunction(this, &AsI18N::I18N_format)));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
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
			ca.args[0].getWideStringSafe()
		);
		break;

	case 2:
		text = i18n::Format(
			ca.args[0].getWideStringSafe(),
			ca.args[1].getWideStringSafe()
		);
		break;

	case 3:
		text = i18n::Format(
			ca.args[0].getWideStringSafe(),
			ca.args[1].getWideStringSafe(),
			ca.args[2].getWideStringSafe()
		);
		break;

	case 4:
		text = i18n::Format(
			ca.args[0].getWideStringSafe(),
			ca.args[1].getWideStringSafe(),
			ca.args[2].getWideStringSafe(),
			ca.args[3].getWideStringSafe()
		);
		break;

	case 5:
		text = i18n::Format(
			ca.args[0].getWideStringSafe(),
			ca.args[1].getWideStringSafe(),
			ca.args[2].getWideStringSafe(),
			ca.args[3].getWideStringSafe(),
			ca.args[4].getWideStringSafe()
		);
		break;
	}

	ca.ret = ActionValue(text);
}

	}
}

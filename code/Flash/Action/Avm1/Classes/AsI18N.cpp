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

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

Ref< ActionObject > AsI18N::alloc(ActionContext* context)
{
	return 0;
}

void AsI18N::init(ActionContext* context, ActionObject* self, const ActionValueArray& args)
{
}

void AsI18N::I18N_format(CallArgs& ca)
{
	std::wstring text;

	switch (ca.args.size())
	{
	case 1:
		text = i18n::Format(
			ca.args[0].getWideString()
		);
		break;

	case 2:
		text = i18n::Format(
			ca.args[0].getWideString(),
			ca.args[1].getWideString()
		);
		break;

	case 3:
		text = i18n::Format(
			ca.args[0].getWideString(),
			ca.args[1].getWideString(),
			ca.args[2].getWideString()
		);
		break;

	case 4:
		text = i18n::Format(
			ca.args[0].getWideString(),
			ca.args[1].getWideString(),
			ca.args[2].getWideString(),
			ca.args[3].getWideString()
		);
		break;

	case 5:
		text = i18n::Format(
			ca.args[0].getWideString(),
			ca.args[1].getWideString(),
			ca.args[2].getWideString(),
			ca.args[3].getWideString(),
			ca.args[4].getWideString()
		);
		break;
	}

	ca.ret = ActionValue(text);
}

	}
}

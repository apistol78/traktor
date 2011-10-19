#include "I18N/Format.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsI18N.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsI18N", AsI18N, ActionClass)

AsI18N::AsI18N(ActionContext* context)
:	ActionClass(context, "I18N")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember("format", ActionValue(createNativeFunction(context, this, &AsI18N::I18N_format)));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void AsI18N::init(ActionObject* self, const ActionValueArray& args) const
{
}

void AsI18N::coerce(ActionObject* self) const
{
	T_FATAL_ERROR;
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

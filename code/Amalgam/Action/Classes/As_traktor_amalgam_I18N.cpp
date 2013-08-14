#include "Amalgam/Action/Classes/As_traktor_amalgam_I18N.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "I18N/Format.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.As_traktor_amalgam_I18N", As_traktor_amalgam_I18N, ActionObject)

As_traktor_amalgam_I18N::As_traktor_amalgam_I18N(flash::ActionContext* context)
:	flash::ActionObject(context)
{
	setMember("format", flash::ActionValue(createNativeFunction(context, this, &As_traktor_amalgam_I18N::I18N_format)));
}

void As_traktor_amalgam_I18N::I18N_format(flash::CallArgs& ca)
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

	ca.ret = flash::ActionValue(text);
}

	}
}

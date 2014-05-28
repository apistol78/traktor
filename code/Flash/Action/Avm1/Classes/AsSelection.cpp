#include "Flash/FlashCharacterInstance.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsSelection.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsSelection", AsSelection, ActionObject)

AsSelection::AsSelection(ActionContext* context)
:	ActionObject(context)
{
	setMember("getBeginIndex", ActionValue(createNativeFunction(context, this, &AsSelection::Selection_getBeginIndex)));
	setMember("getCaretIndex", ActionValue(createNativeFunction(context, this, &AsSelection::Selection_getCaretIndex)));
	setMember("getEndIndex", ActionValue(createNativeFunction(context, this, &AsSelection::Selection_getEndIndex)));
	setMember("getFocus", ActionValue(createNativeFunction(context, this, &AsSelection::Selection_getFocus)));
	setMember("setFocus", ActionValue(createNativeFunction(context, this, &AsSelection::Selection_setFocus)));
	setMember("setSelection", ActionValue(createNativeFunction(context, this, &AsSelection::Selection_setSelection)));
}

void AsSelection::Selection_getBeginIndex(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Selection::getBeginIndex not implemented" << Endl;
	)
}

void AsSelection::Selection_getCaretIndex(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Selection::getCaretIndex not implemented" << Endl;
	)
}

void AsSelection::Selection_getEndIndex(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Selection::getEndIndex not implemented" << Endl;
	)
}

void AsSelection::Selection_getFocus(CallArgs& ca)
{
	FlashCharacterInstance* focusInstance = ca.context->getFocus();
	if (focusInstance)
		ca.ret = ActionValue(focusInstance->getAsObject(ca.context));
}

void AsSelection::Selection_setFocus(CallArgs& ca)
{
	if (ca.args[0].getObject())
	{
		FlashCharacterInstance* focusInstance = ca.args[0].getObject()->getRelay< FlashCharacterInstance >();
		ca.context->setFocus(focusInstance);
	}
	else
		ca.context->setFocus(0);
}

void AsSelection::Selection_setSelection(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Selection::setSelection not implemented" << Endl;
	)
}

	}
}

#include <algorithm>
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Classes/AsMouse.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsMouse", AsMouse, ActionObject)

AsMouse::AsMouse(ActionContext* context)
:	ActionObject(context)
{
	setMember("show", ActionValue(createNativeFunction(context, this, &AsMouse::Mouse_show)));
	setMember("hide", ActionValue(createNativeFunction(context, this, &AsMouse::Mouse_hide)));
}

void AsMouse::eventMouseDown(int x, int y, int button)
{
	ActionValue broadcastMessageValue;
	getMember("broadcastMessage", broadcastMessageValue);

	Ref< ActionFunction > broadcastMessageFn = broadcastMessageValue.getObject< ActionFunction >();
	if (broadcastMessageFn)
	{
		ActionValueArray args(getContext()->getPool(), 1);
		args[0] = ActionValue("onButtonDown");
		broadcastMessageFn->call(this, args);
	}
}

void AsMouse::eventMouseUp(int x, int y, int button)
{
	ActionValue broadcastMessageValue;
	getMember("broadcastMessage", broadcastMessageValue);

	Ref< ActionFunction > broadcastMessageFn = broadcastMessageValue.getObject< ActionFunction >();
	if (broadcastMessageFn)
	{
		ActionValueArray args(getContext()->getPool(), 1);
		args[0] = ActionValue("onButtonUp");
		broadcastMessageFn->call(this, args);
	}
}

void AsMouse::eventMouseMove(int x, int y, int button)
{
	ActionValue broadcastMessageValue;
	getMember("broadcastMessage", broadcastMessageValue);

	Ref< ActionFunction > broadcastMessageFn = broadcastMessageValue.getObject< ActionFunction >();
	if (broadcastMessageFn)
	{
		ActionValueArray args(getContext()->getPool(), 1);
		args[0] = ActionValue("onMouseMove");
		broadcastMessageFn->call(this, args);
	}
}

void AsMouse::eventMouseWheel(int x, int y, int delta)
{
	ActionValue broadcastMessageValue;
	getMember("broadcastMessage", broadcastMessageValue);

	Ref< ActionFunction > broadcastMessageFn = broadcastMessageValue.getObject< ActionFunction >();
	if (broadcastMessageFn)
	{
		ActionValueArray args(getContext()->getPool(), 2);
		args[0] = ActionValue("onMouseWheel");
		args[1] = ActionValue(avm_number_t(delta));
		broadcastMessageFn->call(this, args);
	}
}

void AsMouse::Mouse_show(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Mouse::show not implemented" << Endl;
	)
}

void AsMouse::Mouse_hide(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Mouse::hide not implemented" << Endl;
	)
}

	}
}

#include "Flash/FlashTextFormat.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsTextFormat.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsTextFormat", AsTextFormat, ActionClass)

AsTextFormat::AsTextFormat(ActionContext* context)
:	ActionClass(context, "TextFormat")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void AsTextFormat::initialize(ActionObject* self)
{
}

void AsTextFormat::construct(ActionObject* self, const ActionValueArray& args)
{
	self->setRelay(new FlashTextFormat());
}

ActionValue AsTextFormat::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

	}
}

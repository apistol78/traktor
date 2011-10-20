#include "Core/Log/Log.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsStage.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsStage", AsStage, ActionClass)

AsStage::AsStage(ActionContext* context)
:	ActionClass(context, "Stage")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->addProperty("width", createNativeFunction(context, this, &AsStage::Stage_get_width), 0);
	prototype->addProperty("height", createNativeFunction(context, this, &AsStage::Stage_get_height), 0);

	prototype->setMember("addListener", ActionValue(createNativeFunction(context, this, &AsStage::Stage_addListener)));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void AsStage::init(ActionObject* self, const ActionValueArray& args) const
{
}

void AsStage::coerce(ActionObject* self) const
{
	T_FATAL_ERROR;
}

void AsStage::Stage_get_width(CallArgs& ca)
{
	const FlashMovie* movie = ca.context->getMovie();
	ca.ret = ActionValue((movie->getFrameBounds().max.x - movie->getFrameBounds().min.x) / 20.0f);
}

void AsStage::Stage_get_height(CallArgs& ca)
{
	const FlashMovie* movie = ca.context->getMovie();
	ca.ret = ActionValue((movie->getFrameBounds().max.y - movie->getFrameBounds().min.y) / 20.0f);
}

void AsStage::Stage_addListener(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Stage::addListener not implemented" << Endl;
	)
}

	}
}

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

AsStage::AsStage()
:	ActionClass("Stage")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->addProperty("width", createNativeFunction(this, &AsStage::Stage_get_width), createNativeFunction(this, &AsStage::Stage_set_width));
	prototype->addProperty("height", createNativeFunction(this, &AsStage::Stage_get_height), createNativeFunction(this, &AsStage::Stage_set_height));

	prototype->setMember("addListener", ActionValue(createNativeFunction(this, &AsStage::Stage_addListener)));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

ActionValue AsStage::construct(ActionContext* context, const ActionValueArray& args)
{
	return ActionValue();
}

void AsStage::Stage_get_width(CallArgs& ca)
{
	const FlashMovie* movie = ca.context->getMovie();
	ca.ret = ActionValue((movie->getFrameBounds().max.x - movie->getFrameBounds().min.x) / 20.0f);
}

void AsStage::Stage_set_width(CallArgs& ca)
{
	log::error << L"Stage.width is read-only" << Endl;
}

void AsStage::Stage_get_height(CallArgs& ca)
{
	const FlashMovie* movie = ca.context->getMovie();
	ca.ret = ActionValue((movie->getFrameBounds().max.y - movie->getFrameBounds().min.y) / 20.0f);
}

void AsStage::Stage_set_height(CallArgs& ca)
{
	log::error << L"Stage.height is read-only" << Endl;
}

void AsStage::Stage_addListener(CallArgs& ca)
{
	log::error << L"Stage.addListener not implemented" << Endl;
}

	}
}

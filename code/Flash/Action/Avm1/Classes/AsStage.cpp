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

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsStage", AsStage, ActionObject)

AsStage::AsStage(ActionContext* context)
:	ActionObject(context)
{
	addProperty("width", createNativeFunction(context, this, &AsStage::Stage_get_width), 0);
	addProperty("height", createNativeFunction(context, this, &AsStage::Stage_get_height), 0);
	setMember("addListener", ActionValue(createNativeFunction(context, this, &AsStage::Stage_addListener)));
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

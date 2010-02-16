#include "Core/Log/Log.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsStage.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsStage", AsStage, ActionClass)

Ref< AsStage > AsStage::getInstance()
{
	static Ref< AsStage > instance = 0;
	if (!instance)
	{
		instance = new AsStage();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

AsStage::AsStage()
:	ActionClass(L"Stage")
{
}

void AsStage::createPrototype()
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"__proto__", ActionValue(AsObject::getInstance()));

	prototype->addProperty(L"width", createNativeFunction(this, &AsStage::Stage_get_width), createNativeFunction(this, &AsStage::Stage_set_width));
	prototype->addProperty(L"height", createNativeFunction(this, &AsStage::Stage_get_height), createNativeFunction(this, &AsStage::Stage_set_height));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
}

ActionValue AsStage::construct(ActionContext* context, const args_t& args)
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

	}
}

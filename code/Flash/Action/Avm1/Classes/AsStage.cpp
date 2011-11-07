#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
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
,	m_alignH(SaCenter)
,	m_alignV(SaCenter)
,	m_scaleMode(SmShowAll)
{
	addProperty("align", createNativeFunction(context, this, &AsStage::Stage_get_align), createNativeFunction(context, this, &AsStage::Stage_set_align));
	addProperty("height", createNativeFunction(context, this, &AsStage::Stage_get_height), 0);
	addProperty("scaleMode", createNativeFunction(context, this, &AsStage::Stage_get_scaleMode), createNativeFunction(context, this, &AsStage::Stage_set_scaleMode));
	addProperty("showMenu", createNativeFunction(context, this, &AsStage::Stage_get_showMenu), createNativeFunction(context, this, &AsStage::Stage_set_showMenu));
	addProperty("width", createNativeFunction(context, this, &AsStage::Stage_get_width), 0);
}

void AsStage::eventResize()
{
	ActionValue broadcastMessageValue;
	getMember("broadcastMessage", broadcastMessageValue);

	Ref< ActionFunction > broadcastMessageFn = broadcastMessageValue.getObject< ActionFunction >();
	if (broadcastMessageFn)
	{
		ActionValueArray args(getContext()->getPool(), 1);
		args[0] = ActionValue("onResize");
		broadcastMessageFn->call(this, args);
	}
}

void AsStage::Stage_get_align(CallArgs& ca)
{
	const char* tbl[3][3] =
	{
		{ "TL", "T", "TR" },
		{ "L", "", "R" },
		{ "BR", "B", "BR" }
	};
	ca.ret = ActionValue(tbl[m_alignV][m_alignH]);
}

void AsStage::Stage_set_align(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Stage::set_align not implemented" << Endl;
	)
}

void AsStage::Stage_get_height(CallArgs& ca)
{
	const FlashMovie* movie = ca.context->getMovie();
	ca.ret = ActionValue((movie->getFrameBounds().max.y - movie->getFrameBounds().min.y) / 20.0f);
}

void AsStage::Stage_get_scaleMode(CallArgs& ca)
{
	const char* tbl[] =
	{
		"showAll",
		"noBorder",
		"exactFit",
		"noScale"
	};
	ca.ret = ActionValue(tbl[m_scaleMode]);
}

void AsStage::Stage_set_scaleMode(CallArgs& ca)
{
	std::string sm = ca.args[0].getString();
	if (compareIgnoreCase< std::string >(sm, "showAll") == 0)
		m_scaleMode = SmShowAll;
	else if (compareIgnoreCase< std::string >(sm, "noBorder") == 0)
		m_scaleMode = SmNoBorder;
	else if (compareIgnoreCase< std::string >(sm, "exactFit") == 0)
		m_scaleMode = SmExactFit;
	else if (compareIgnoreCase< std::string >(sm, "noScale") == 0)
		m_scaleMode = SmNoScale;
}

void AsStage::Stage_get_showMenu(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Stage::get_showMenu not implemented" << Endl;
	)
}

void AsStage::Stage_set_showMenu(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Stage::set_showMenu not implemented" << Endl;
	)
}

void AsStage::Stage_get_width(CallArgs& ca)
{
	const FlashMovie* movie = ca.context->getMovie();
	ca.ret = ActionValue((movie->getFrameBounds().max.x - movie->getFrameBounds().min.x) / 20.0f);
}


	}
}

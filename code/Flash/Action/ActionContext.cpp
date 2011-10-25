#include "Core/Misc/StringSplit.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/Action/ActionContext.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionContext", ActionContext, Collectable)

ActionContext::ActionContext(const IActionVM* vm, const FlashMovie* movie)
:	m_vm(vm)
,	m_movie(movie)
{
}

void ActionContext::setGlobal(ActionObject* global)
{
	m_global = global;
}

void ActionContext::setMovieClip(FlashSpriteInstance* movieClip)
{
	m_movieClip = movieClip;
}

ActionObject* ActionContext::lookupClass(const std::string& className)
{
	Ref< ActionObject > clazz = m_global;

	StringSplit< std::string > classNameSplit(className, ".");
	for (StringSplit< std::string >::const_iterator i = classNameSplit.begin(); i != classNameSplit.end(); ++i)
	{
		ActionValue clazzMember;
		if (!clazz->getLocalMember(*i, clazzMember) || !clazzMember.isObject())
			return 0;

		clazz = clazzMember.getObject();
	}

	if (!clazz)
		return 0;

	ActionValue prototypeMember;
	if (!clazz->getLocalMember("prototype", prototypeMember) || !prototypeMember.isObject())
		return 0;

	return prototypeMember.getObject();
}

void ActionContext::trace(const IVisitor& visitor) const
{
	visitor(m_global);
	visitor(m_movieClip);
}

void ActionContext::dereference()
{
	m_global = 0;
	m_movieClip = 0;
}

	}
}

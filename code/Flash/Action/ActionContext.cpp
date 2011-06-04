#include "Core/Misc/StringSplit.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/ActionValueArray.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionContext", ActionContext, Object)

ActionContext::ActionContext(const IActionVM* vm, const FlashMovie* movie, ActionObject* global)
:	m_vm(vm)
,	m_movie(movie)
,	m_global(global)
{
}

void ActionContext::addFrameListener(ActionObject* frameListener)
{
	ActionValue memberValue;
	if (frameListener->getLocalMember("onFrame", memberValue))
	{
		Ref< ActionFunction > listenerFunction = memberValue.getObject< ActionFunction >();
		if (listenerFunction)
		{
			FrameListener fl = { frameListener, listenerFunction };
			m_frameListeners.push_back(fl);
		}
	}
}

void ActionContext::removeFrameListener(ActionObject* frameListener)
{
	for (std::vector< FrameListener >::iterator i = m_frameListeners.begin(); i != m_frameListeners.end(); ++i)
	{
		if (i->listenerTarget == frameListener)
		{
			m_frameListeners.erase(i);
			break;
		}
	}
}

void ActionContext::notifyFrameListeners(avm_number_t time)
{
	if (m_frameListeners.empty())
		return;

	ActionValueArray argv(m_pool, 1);
	argv[0] = ActionValue(time);

	std::vector< FrameListener > frameListeners = m_frameListeners;
	for (std::vector< FrameListener >::iterator i = frameListeners.begin(); i != frameListeners.end(); ++i)
		i->listenerFunction->call(this, i->listenerTarget, argv);
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

	}
}

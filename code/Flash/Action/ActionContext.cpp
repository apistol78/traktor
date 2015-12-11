#include "Core/Misc/StringSplit.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunction.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionContext", ActionContext, Collectable)

ActionContext::ActionContext(const FlashMovie* movie, const IFlashMovieLoader* movieLoader)
:	m_movie(movie)
,	m_movieLoader(movieLoader)
{
	m_strings[""];	// 0
	m_strings["__proto__"];
	m_strings["prototype"];
	m_strings["__ctor__"];
	m_strings["constructor"];
	m_strings["this"];
	m_strings["super"];
	m_strings["_global"];
	m_strings["arguments"];
	m_strings["_root"];
	m_strings["_parent"];
	m_strings["apply"];
	m_strings["call"];
	m_strings["toString"];
	m_strings["_target"];
	m_strings["_targetProperty"];
	m_strings["_function"];
	m_strings["onLoad"];
	m_strings["onEnterFrame"];
	m_strings["onKeyDown"];
	m_strings["onKeyUp"];
	m_strings["onMouseDown"];
	m_strings["onPress"];
	m_strings["onMouseUp"];
	m_strings["onRelease"];
	m_strings["onMouseMove"];
	m_strings["onRollOver"];
	m_strings["onRollOut"];
	m_strings["onFrame"];
	m_strings["onSetFocus"];
	m_strings["onKillFocus"];
	m_strings["onMouseWheel"];
	m_strings["onChanged"];
	m_strings["onScroller"];
}

void ActionContext::setGlobal(ActionObject* global)
{
	m_global = global;
}

void ActionContext::setMovieClip(FlashSpriteInstance* movieClip)
{
	m_movieClip = movieClip;
}

void ActionContext::setFocus(FlashCharacterInstance* focus)
{
	if (m_focus)
		m_focus->eventKillFocus();

	m_focus = focus;

	if (m_focus)
		m_focus->eventSetFocus();
}

void ActionContext::setPressed(FlashCharacterInstance* pressed)
{
	m_pressed = pressed;
}

void ActionContext::addFrameListener(ActionObject* frameListener)
{
	ActionValue memberValue;
	if (frameListener->getLocalMember(IdOnFrame, memberValue))
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
		i->listenerFunction->call(i->listenerTarget, argv);
}

ActionObject* ActionContext::lookupClass(const std::string& className)
{
	Ref< ActionObject > clazz = m_global;

	StringSplit< std::string > classNameSplit(className, ".");
	for (StringSplit< std::string >::const_iterator i = classNameSplit.begin(); clazz != 0 && i != classNameSplit.end(); ++i)
	{
		ActionValue clazzMember;
		if (!clazz->getLocalMember(m_strings[*i], clazzMember) || !clazzMember.isObject())
			return 0;

		clazz = clazzMember.getObject();
	}

	if (!clazz)
		return 0;

	ActionValue prototypeMember;
	if (!clazz->getLocalMember(IdPrototype, prototypeMember) || !prototypeMember.isObject())
		return 0;

	return prototypeMember.getObject();
}

uint32_t ActionContext::getString(const std::string& str)
{
	return m_strings[str];
}

std::string ActionContext::getString(uint32_t id)
{
	return m_strings[id];
}

void ActionContext::trace(visitor_t visitor) const
{
	T_ASSERT (m_pool.offset() == 0);

	visitor(m_global);
	visitor(m_movieClip);
	visitor(m_focus);

	for (std::vector< FrameListener >::const_iterator i = m_frameListeners.begin(); i != m_frameListeners.end(); ++i)
	{
		visitor(i->listenerTarget);
		visitor(i->listenerFunction);
	}
}

void ActionContext::dereference()
{
	m_global = 0;
	m_movieClip = 0;
	m_focus = 0;
	m_pressed = 0;
	m_frameListeners.clear();
}

	}
}

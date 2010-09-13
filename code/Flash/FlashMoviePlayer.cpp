#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Math/Const.h"
#include "Core/Timer/Timer.h"
#include "Flash/FlashMoviePlayer.h"
#include "Flash/FlashMovieRenderer.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionFunctionNative.h"

// ActionScript VM 1
#include "Flash/Action/Avm1/ActionVM1.h"
#include "Flash/Action/Avm1/Classes/AsKey.h"
#include "Flash/Action/Avm1/Classes/AsMouse.h"

// ActionScript VM 2
#include "Flash/Action/Avm2/ActionVM2.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashMoviePlayer", FlashMoviePlayer, Object)

FlashMoviePlayer::FlashMoviePlayer(IDisplayRenderer* displayRenderer)
:	m_displayRenderer(displayRenderer)
,	m_movieRenderer(new FlashMovieRenderer(displayRenderer))
,	m_intervalNextId(1)
,	m_timeCurrent(0.0)
,	m_timeNext(0.0)
,	m_timeNextFrame(0.0)
{
}

FlashMoviePlayer::~FlashMoviePlayer()
{
	T_EXCEPTION_GUARD_BEGIN

	destroy();

	T_EXCEPTION_GUARD_END
}

bool FlashMoviePlayer::create(FlashMovie* movie)
{
	ActionValue memberValue;

	// Create ActionScript virtual machine.
	if (1)
		m_actionVM = new ActionVM1();
	else
		m_actionVM = new ActionVM2();

	m_movie = movie;
	m_movieInstance = m_movie->createMovieClipInstance(m_actionVM);

	// Override some global methods.
	setGlobal(L"getUrl", ActionValue(createNativeFunction(this, &FlashMoviePlayer::Global_getUrl)));
	setGlobal(L"setInterval", ActionValue(createNativeFunction(this, &FlashMoviePlayer::Global_setInterval)));
	setGlobal(L"clearInterval", ActionValue(createNativeFunction(this, &FlashMoviePlayer::Global_clearInterval)));

	// Get references to key and mouse singletons.
	Ref< ActionContext > context = m_movieInstance->getContext();
	Ref< ActionObject > global = context->getGlobal();

	Ref< AsKey > key;
	if (global->getMember(L"Key", memberValue))
		m_key = memberValue.getObject< AsKey >();

	Ref< AsMouse > mouse;
	if (global->getMember(L"Mouse", memberValue))
		m_mouse = memberValue.getObject< AsMouse >();

	return true;
}

void FlashMoviePlayer::destroy()
{
	m_displayRenderer = 0;
	m_movieRenderer = 0;
	m_actionVM = 0;
	m_key = 0;
	m_mouse = 0;
	m_movie = 0;
	
	if (m_movieInstance)
	{
		m_movieInstance->destroy();
		m_movieInstance = 0;
	}
	
	m_events.clear();
	m_fsCommands.clear();
	m_interval.clear();
}

void FlashMoviePlayer::gotoAndPlay(uint32_t frame)
{
	m_movieInstance->setPlaying(true);
	m_movieInstance->gotoFrame(frame);
}

void FlashMoviePlayer::gotoAndStop(uint32_t frame)
{
	m_movieInstance->setPlaying(false);
	m_movieInstance->gotoFrame(frame);
}

void FlashMoviePlayer::gotoAndPlay(const std::wstring& frameLabel)
{
	int frame = m_movie->getMovieClip()->findFrame(frameLabel);
	if (frame >= 0)
	{
		m_movieInstance->setPlaying(true);
		m_movieInstance->gotoFrame(frame);
	}
}

void FlashMoviePlayer::gotoAndStop(const std::wstring& frameLabel)
{
	int frame = m_movie->getMovieClip()->findFrame(frameLabel);
	if (frame >= 0)
	{
		m_movieInstance->setPlaying(false);
		m_movieInstance->gotoFrame(frame);
	}
}

uint32_t FlashMoviePlayer::getFrameCount() const
{
	return m_movie->getMovieClip()->getFrameCount();
}

void FlashMoviePlayer::renderFrame()
{
	m_movieRenderer->renderFrame(m_movie, m_movieInstance);
}

void FlashMoviePlayer::executeFrame()
{
	Ref< ActionContext > context = m_movieInstance->getContext();
	ActionValue memberValue;

	// Issue interval functions.
	for (std::map< uint32_t, Interval >::iterator i = m_interval.begin(); i != m_interval.end(); ++i)
		i->second.function->call(context, i->second.target, ActionValueArray());

	// Issue all events in sequence as each event possibly update
	// the play head and other aspects of the movie.

	m_movieInstance->updateDisplayList();

	m_movieInstance->preDispatchEvents();

	while (!m_events.empty())
	{
		const Event& evt = m_events.front();
		switch (evt.eventType)
		{
		case EvtKeyDown:
			m_movieInstance->eventKeyDown(evt.keyCode);
			if (m_key)
				m_key->eventKeyDown(context, evt.keyCode);
			break;

		case EvtKeyUp:
			m_movieInstance->eventKeyUp(evt.keyCode);
			if (m_key)
				m_key->eventKeyUp(context, evt.keyCode);
			break;

		case EvtMouseDown:
			m_movieInstance->eventMouseDown(evt.mouse.x, evt.mouse.y, evt.mouse.button);
			if (m_mouse)
				m_mouse->eventMouseDown(context, evt.mouse.x, evt.mouse.y, evt.mouse.button);
			break;

		case EvtMouseUp:
			m_movieInstance->eventMouseUp(evt.mouse.x, evt.mouse.y, evt.mouse.button);
			if (m_mouse)
				m_mouse->eventMouseUp(context, evt.mouse.x, evt.mouse.y, evt.mouse.button);
			break;

		case EvtMouseMove:
			m_movieInstance->eventMouseMove(evt.mouse.x, evt.mouse.y, evt.mouse.button);
			if (m_mouse)
				m_mouse->eventMouseMove(context, evt.mouse.x, evt.mouse.y, evt.mouse.button);
			break;
		}
		m_events.pop_front();
	}

	// Finally issue the frame event.
	m_movieInstance->eventFrame();

	// Notify frame listeners.
	context->notifyFrameListeners(avm_number_t(m_timeCurrent));

	m_movieInstance->postDispatchEvents();

	// Flush pool memory; release all lingering object references etc.
	context->getPool().flush();
}

bool FlashMoviePlayer::progressFrame(float deltaTime)
{
	bool executed = false;

	if (m_timeNext >= m_timeNextFrame)
	{
		m_timeCurrent = m_timeNext;
		m_timeNextFrame += 1.0 / m_movie->getMovieClip()->getFrameRate();

		executeFrame();

		executed = true;
	}

	m_timeNext += deltaTime;

	return executed;
}

void FlashMoviePlayer::postKeyDown(int keyCode)
{
	Event evt;
	evt.eventType = EvtKeyDown;
	evt.keyCode = keyCode;
	m_events.push_back(evt);
}

void FlashMoviePlayer::postKeyUp(int keyCode)
{
	Event evt;
	evt.eventType = EvtKeyUp;
	evt.keyCode = keyCode;
	m_events.push_back(evt);
}

void FlashMoviePlayer::postMouseDown(int x, int y, int button)
{
	Event evt;
	evt.eventType = EvtMouseDown;
	evt.mouse.x = x;
	evt.mouse.y = y;
	evt.mouse.button = button;
	m_events.push_back(evt);
}

void FlashMoviePlayer::postMouseUp(int x, int y, int button)
{
	Event evt;
	evt.eventType = EvtMouseUp;
	evt.mouse.x = x;
	evt.mouse.y = y;
	evt.mouse.button = button;
	m_events.push_back(evt);
}

void FlashMoviePlayer::postMouseMove(int x, int y, int button)
{
	Event evt;
	evt.eventType = EvtMouseMove;
	evt.mouse.x = x;
	evt.mouse.y = y;
	evt.mouse.button = button;
	m_events.push_back(evt);
}

bool FlashMoviePlayer::getFsCommand(std::wstring& outCommand, std::wstring& outArgs)
{
	if (m_fsCommands.empty())
		return false;

	outCommand = m_fsCommands.front().first;
	outArgs = m_fsCommands.front().second;

	m_fsCommands.pop_front();
	return true;
}

FlashSpriteInstance* FlashMoviePlayer::getMovieInstance() const
{
	return m_movieInstance;
}

IActionVM* FlashMoviePlayer::getVM() const
{
	return m_actionVM;
}

void FlashMoviePlayer::setGlobal(const std::wstring& name, const ActionValue& value)
{
	ActionContext* actionContext = m_movieInstance->getContext();
	T_ASSERT (actionContext);

	ActionObject* global = actionContext->getGlobal();
	T_ASSERT (global);

	global->setMember(name, value);
}

ActionValue FlashMoviePlayer::getGlobal(const std::wstring& name) const
{
	ActionContext* actionContext = m_movieInstance->getContext();
	T_ASSERT (actionContext);

	ActionObject* global = actionContext->getGlobal();
	T_ASSERT (global);

	ActionValue value;
	global->getMember(name, value);

	return value;
}

void FlashMoviePlayer::Global_getUrl(CallArgs& ca)
{
	std::wstring url = ca.args[0].getString();
	if (startsWith(url, L"FSCommand:"))
	{
		m_fsCommands.push_back(std::make_pair(
			url.substr(10),
			ca.args[1].getStringSafe()
		));
	}
}

void FlashMoviePlayer::Global_setInterval(CallArgs& ca)
{
	Ref< ActionObject > target;
	Ref< ActionFunction > function;
	ActionValue functionValue;
	uint32_t interval;

	if (ca.args[1].isString())
	{
		// (objectReference:Object, methodName:String, interval:Number, [param1:Object, param2, ..., paramN])
		target = ca.args[0].getObjectSafe();
		if (!target->getMember(ca.args[1].getString(), functionValue))
			return;
		function = functionValue.getObject< ActionFunction >();
		interval = uint32_t(ca.args[2].getNumberSafe());
	}
	else
	{
		// (functionReference:Function, interval:Number, [param1:Object, param2, ..., paramN])
		return;
	}

	if (!function)
		return;

	uint32_t id = m_intervalNextId++;

	Interval& iv = m_interval[id];
	iv.last = 0;
	iv.interval = interval;
	iv.target = target;
	iv.function = function;

	ca.ret = ActionValue(avm_number_t(id));
}

void FlashMoviePlayer::Global_clearInterval(CallArgs& ca)
{
	uint32_t id = uint32_t(ca.args[0].getNumberSafe());
	std::map< uint32_t, Interval >::iterator i = m_interval.find(id);
	if (i != m_interval.end())
		m_interval.erase(i);
}

	}
}

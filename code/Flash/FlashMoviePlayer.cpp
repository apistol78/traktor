#include "Flash/FlashMoviePlayer.h"
#include "Flash/FlashMovieRenderer.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/Action/ActionVM.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Classes/AsKey.h"
#include "Flash/Action/Classes/AsMouse.h"
#include "Core/Thread/Acquire.h"
#include "Core/Timer/Timer.h"
#include "Core/Math/Const.h"
#include "Core/Misc/String.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashMoviePlayer", FlashMoviePlayer, Object)

FlashMoviePlayer::FlashMoviePlayer(DisplayRenderer* displayRenderer)
:	m_displayRenderer(displayRenderer)
,	m_movieRenderer(gc_new< FlashMovieRenderer >(displayRenderer))
,	m_actionVM(gc_new< ActionVM >())
,	m_intervalNextId(1)
,	m_untilNextFrame(0.0f)
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

	m_movie = movie;
	m_movieInstance = m_movie->createMovieClipInstance();

	Ref< ActionContext > context = m_movieInstance->getContext();
	Ref< ActionObject > global = context->getGlobal();

	// Override some global methods.
	global->setMember(L"getUrl", createNativeFunctionValue(this, &FlashMoviePlayer::Global_getUrl));
	global->setMember(L"setInterval", createNativeFunctionValue(this, &FlashMoviePlayer::Global_setInterval));
	global->setMember(L"clearInterval", createNativeFunctionValue(this, &FlashMoviePlayer::Global_clearInterval));

	// Get references to key and mouse singletons.
	Ref< AsKey > key;
	if (context->getGlobal()->getMember(L"Key", memberValue))
		m_key = memberValue.getObject< AsKey >();

	Ref< AsMouse > mouse;
	if (context->getGlobal()->getMember(L"Mouse", memberValue))
		m_mouse = memberValue.getObject< AsMouse >();

	return true;
}

void FlashMoviePlayer::destroy()
{
	m_displayRenderer = 0;
	m_movieRenderer = 0;
	m_actionVM = 0;
	m_movie = 0;
	m_movieInstance = 0;
	m_events.clear();
	m_fsCommands.clear();
}

void FlashMoviePlayer::gotoAndPlay(uint32_t frame)
{
	Acquire< Semaphore > lock(m_renderLock);
	m_movieInstance->setPlaying(true);
	m_movieInstance->gotoFrame(frame);
}

void FlashMoviePlayer::gotoAndStop(uint32_t frame)
{
	Acquire< Semaphore > lock(m_renderLock);
	m_movieInstance->setPlaying(false);
	m_movieInstance->gotoFrame(frame);
}

void FlashMoviePlayer::gotoAndPlay(const std::wstring& frameLabel)
{
	Acquire< Semaphore > lock(m_renderLock);
	int frame = m_movie->getMovieClip()->findFrame(frameLabel);
	if (frame >= 0)
	{
		m_movieInstance->setPlaying(true);
		m_movieInstance->gotoFrame(frame);
	}
}

void FlashMoviePlayer::gotoAndStop(const std::wstring& frameLabel)
{
	Acquire< Semaphore > lock(m_renderLock);
	int frame = m_movie->getMovieClip()->findFrame(frameLabel);
	if (frame >= 0)
	{
		m_movieInstance->setPlaying(false);
		m_movieInstance->gotoFrame(frame);
	}
}

uint32_t FlashMoviePlayer::getFrameCount() const
{
	Acquire< Semaphore > lock(m_renderLock);
	return m_movie->getMovieClip()->getFrameCount();
}

void FlashMoviePlayer::renderFrame()
{
	Acquire< Semaphore > lock(m_renderLock);
	m_movieRenderer->renderFrame(m_movie, m_movieInstance);
}

void FlashMoviePlayer::executeFrame()
{
	Acquire< Semaphore > lock(m_renderLock);

	Ref< ActionContext > context = m_movieInstance->getContext();
	ActionValue memberValue;

	// Issue interval functions.
	for (std::map< uint32_t, Interval >::iterator i = m_interval.begin(); i != m_interval.end(); ++i)
	{
		ActionFrame callFrame(context, 0, 0, 0, 4, 0, 0);
		i->second.function->call(
			m_actionVM,
			&callFrame,
			i->second.target
		);
	}

	// Issue all events in sequence as each event possibly update
	// the play head and other aspects of the movie.

	m_movieInstance->updateDisplayList();

	m_movieInstance->preDispatchEvents(m_actionVM);

	while (!m_events.empty())
	{
		const Event& evt = m_events.front();
		switch (evt.eventType)
		{
		case EvtKeyDown:
			m_movieInstance->eventKeyDown(m_actionVM, evt.keyCode);
			if (m_key)
				m_key->eventKeyDown(m_actionVM, context, evt.keyCode);
			break;

		case EvtKeyUp:
			m_movieInstance->eventKeyUp(m_actionVM, evt.keyCode);
			if (m_key)
				m_key->eventKeyUp(m_actionVM, context, evt.keyCode);
			break;

		case EvtMouseDown:
			m_movieInstance->eventMouseDown(m_actionVM, evt.mouse.x, evt.mouse.y, evt.mouse.button);
			if (m_mouse)
				m_mouse->eventMouseDown(m_actionVM, context, evt.mouse.x, evt.mouse.y, evt.mouse.button);
			break;

		case EvtMouseUp:
			m_movieInstance->eventMouseUp(m_actionVM, evt.mouse.x, evt.mouse.y, evt.mouse.button);
			if (m_mouse)
				m_mouse->eventMouseUp(m_actionVM, context, evt.mouse.x, evt.mouse.y, evt.mouse.button);
			break;

		case EvtMouseMove:
			m_movieInstance->eventMouseMove(m_actionVM, evt.mouse.x, evt.mouse.y, evt.mouse.button);
			if (m_mouse)
				m_mouse->eventMouseMove(m_actionVM, context, evt.mouse.x, evt.mouse.y, evt.mouse.button);
			break;
		}
		m_events.pop_front();
	}

	// Finally issue the frame event.
	m_movieInstance->eventFrame(m_actionVM);

	m_movieInstance->postDispatchEvents(m_actionVM);
}

bool FlashMoviePlayer::progressFrame(float deltaTime)
{
	m_untilNextFrame -= deltaTime;
	if (m_untilNextFrame <= FUZZY_EPSILON)
	{
		Timer timer;
		timer.start();
		
		executeFrame();

		m_untilNextFrame = 1.0f / m_movie->getMovieClip()->getFrameRate();
		m_untilNextFrame -= float(timer.getElapsedTime());
		return true;
	}
	else
		return false;
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

	ca.ret = ActionValue(double(id));
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

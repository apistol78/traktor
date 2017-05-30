/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Math/Const.h"
#include "Core/System/OS.h"
#include "Core/Timer/Profiler.h"
#include "Core/Timer/Timer.h"
#include "Flash/Dictionary.h"
#include "Flash/MoviePlayer.h"
#include "Flash/MovieRenderer.h"
#include "Flash/Movie.h"
#include "Flash/SoundPlayer.h"
#include "Flash/Sprite.h"
#include "Flash/SpriteInstance.h"
#include "Flash/GC.h"
#include "Flash/IDisplayRenderer.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Classes/AsKey.h"
#include "Flash/Action/Common/Classes/AsMouse.h"
#include "Flash/Action/Common/Classes/AsSound.h"
#include "Flash/Action/Common/Classes/AsStage.h"
#include "Flash/Action/Common/Classes/As_flash_external_ExternalInterface.h"
#include "Flash/Debug/MovieDebugger.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

const int32_t c_framesBetweenCollections = 100;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.MoviePlayer", MoviePlayer, Object)

MoviePlayer::MoviePlayer(
	IDisplayRenderer* displayRenderer,
	ISoundRenderer* soundRenderer,
	const ICharacterFactory* characterFactory,
	const IMovieLoader* movieLoader,
	const MovieDebugger* movieDebugger
)
:	m_displayRenderer(displayRenderer)
,	m_soundRenderer(soundRenderer)
,	m_characterFactory(characterFactory)
,	m_movieLoader(movieLoader)
,	m_movieDebugger(movieDebugger)
,	m_movieRenderer(new MovieRenderer(displayRenderer))
,	m_intervalNextId(1)
,	m_timeCurrent(0.0f)
,	m_timeNext(0.0f)
,	m_timeNextFrame(0.0f)
,	m_gcEnable(true)
,	m_framesUntilCollection(c_framesBetweenCollections)
{
	if (soundRenderer)
		m_soundPlayer = new SoundPlayer(soundRenderer);
}

MoviePlayer::~MoviePlayer()
{
	T_EXCEPTION_GUARD_BEGIN

	destroy();

	T_EXCEPTION_GUARD_END
}

bool MoviePlayer::create(Movie* movie, int32_t width, int32_t height)
{
	ActionValue memberValue;

	m_movie = movie;
	m_movieInstance = m_movie->createMovieClipInstance(m_characterFactory, m_movieLoader);

	Ref< ActionContext > context = m_movieInstance->getContext();
	Ref< ActionObject > global = context->getGlobal();

	// Override some global methods.
	setGlobal("getURL", ActionValue(createNativeFunction(context, this, &MoviePlayer::Global_getURL)));
	setGlobal("setInterval", ActionValue(createNativeFunction(context, this, &MoviePlayer::Global_setInterval)));
	setGlobal("clearInterval", ActionValue(createNativeFunction(context, this, &MoviePlayer::Global_clearInterval)));

	// Create sound prototype.
	setGlobal("Sound", ActionValue(new AsSound(context, m_soundPlayer)));

	// Get references to key and mouse singletons.
	if (global->getMemberByQName("Key", memberValue))
		m_key = memberValue.getObject< AsKey >();
	if (global->getMemberByQName("Mouse", memberValue))
		m_mouse = memberValue.getObject< AsMouse >();
	if (global->getMemberByQName("Stage", memberValue))
		m_stage = memberValue.getObject< AsStage >();
	if (global->getMemberByQName("flash.external.ExternalInterface", memberValue))
		m_externalInterface = memberValue.getObject< As_flash_external_ExternalInterface >();
	
	// Ensure stage are properly initialized.
	if (m_stage)
		m_stage->eventResize(width, height);

	return true;
}

void MoviePlayer::destroy()
{
	if (m_movieInstance)
	{
		// Ensure no character have focus; need to do this
		// as the focus character will trigger "exit focus" event
		// thus need to have access to context etc.
		ActionContext* context = m_movieInstance->getContext();
		T_ASSERT (context);
		context->setFocus(0);
		context->setPressed(0);

		// Then destroy root movie instance.
		safeDestroy(m_movieInstance);
	}

	m_displayRenderer = 0;
	m_soundRenderer = 0;
	m_movieRenderer = 0;
	m_soundPlayer = 0;
	m_actionVM = 0;
	m_externalInterface = 0;
	m_stage =  0;
	m_key = 0;
	m_mouse = 0;
	m_movie = 0;

	m_events.clear();
	m_fsCommands.clear();
	m_interval.clear();

	GC::getInstance().collectCycles(true);
}

void MoviePlayer::gotoAndPlay(uint32_t frame)
{
	m_movieInstance->setPlaying(true, false);
	m_movieInstance->gotoFrame(frame);
}

void MoviePlayer::gotoAndStop(uint32_t frame)
{
	m_movieInstance->setPlaying(false, false);
	m_movieInstance->gotoFrame(frame);
}

bool MoviePlayer::gotoAndPlay(const std::string& frameLabel)
{
	int frame = m_movie->getMovieClip()->findFrame(frameLabel);
	if (frame < 0)
		return false;

	m_movieInstance->setPlaying(true, false);
	m_movieInstance->gotoFrame(frame);
	
	return true;
}

bool MoviePlayer::gotoAndStop(const std::string& frameLabel)
{
	int frame = m_movie->getMovieClip()->findFrame(frameLabel);
	if (frame < 0)
		return false;

	m_movieInstance->setPlaying(false, false);
	m_movieInstance->gotoFrame(frame);

	return true;
}

uint32_t MoviePlayer::getFrameCount() const
{
	return m_movie->getMovieClip()->getFrameCount();
}

void MoviePlayer::renderFrame()
{
	m_movieRenderer->renderFrame(
		m_movieInstance,
		m_movie->getFrameBounds(),
		m_stage->getFrameTransform(),
		float(m_stage->getViewWidth()),
		float(m_stage->getViewHeight())
	);
}

void MoviePlayer::executeFrame()
{
	T_PROFILER_SCOPE(L"MoviePlayer executeFrame");

	ActionContext* context = m_movieInstance->getContext();
	T_ASSERT (context);

	Ref< SpriteInstance > current = context->getMovieClip();
	context->setMovieClip(m_movieInstance);

	// Collect and issue interval functions.
	if (!m_interval.empty())
	{
		T_PROFILER_SCOPE(L"MoviePlayer interval");
		AlignedVector< std::pair< ActionObject*, ActionFunction* > > intervalFns;
		for (std::map< uint32_t, Interval >::iterator i = m_interval.begin(); i != m_interval.end(); ++i)
		{
			if (i->second.count++ >= i->second.interval)
			{
				intervalFns.push_back(std::make_pair(i->second.target, i->second.function));
				i->second.count = 0;
			}
		}
		ActionValueArray argv;
		for (AlignedVector< std::pair< ActionObject*, ActionFunction* > >::const_iterator i = intervalFns.begin(); i != intervalFns.end(); ++i)
			i->second->call(i->first, argv);
	}

	// Issue all events in sequence as each event possibly update
	// the play head and other aspects of the movie.
	{
		T_PROFILER_SCOPE(L"MoviePlayer updateDisplayList");
		if (m_soundPlayer)
			m_movieInstance->updateDisplayListAndSounds(m_soundPlayer);
		else
			m_movieInstance->updateDisplayList();
	}

	// Dispatch events.
	if (!m_events.empty())
	{
		T_PROFILER_SCOPE(L"MoviePlayer events");
		for (AlignedVector< Event >::const_iterator i = m_events.begin(); i != m_events.end(); ++i)
		{
			const Event& evt = *i;
			switch (evt.eventType)
			{
			case EvtKey:
				m_movieInstance->eventKey(evt.unicode);
				break;

			case EvtKeyDown:
				if (m_key)
					m_key->eventKeyDown(evt.keyCode);
				m_movieInstance->eventKeyDown(evt.keyCode);
				break;

			case EvtKeyUp:
				if (m_key)
					m_key->eventKeyUp(evt.keyCode);
				m_movieInstance->eventKeyUp(evt.keyCode);
				break;

			case EvtMouseDown:
				if (m_mouse)
					m_mouse->eventMouseDown(evt.mouse.x, evt.mouse.y, evt.mouse.button);
				m_movieInstance->eventMouseDown(evt.mouse.x, evt.mouse.y, evt.mouse.button);
				break;

			case EvtMouseUp:
				if (m_mouse)
					m_mouse->eventMouseUp(evt.mouse.x, evt.mouse.y, evt.mouse.button);
				m_movieInstance->eventMouseUp(evt.mouse.x, evt.mouse.y, evt.mouse.button);
				break;

			case EvtMouseMove:
				{
					if (m_mouse)
						m_mouse->eventMouseMove(evt.mouse.x, evt.mouse.y, evt.mouse.button);

					// Check if mouse cursor still over "rolled over" character.
					if (context->getRolledOver())
					{
						SpriteInstance* rolledOverSprite = checked_type_cast< SpriteInstance* >(context->getRolledOver());
						bool inside = false;
						if (rolledOverSprite->isVisible())
						{
							Matrix33 intoCharacter = rolledOverSprite->getFullTransform().inverse();
							Aabb2 bounds = rolledOverSprite->getVisibleLocalBounds();
							Vector2 xy = intoCharacter * Vector2(float(evt.mouse.x), float(evt.mouse.y));
							inside = (xy.x >= bounds.mn.x && xy.y >= bounds.mn.y && xy.x <= bounds.mx.x && xy.y <= bounds.mx.y);
						}
						if (!inside)
						{
							// Cursor has escaped; issue roll out.
							rolledOverSprite->executeScriptEvent(ActionContext::IdOnRollOut, ActionValue());
							context->setRolledOver(0);
						}
					}

					m_movieInstance->eventMouseMove(evt.mouse.x, evt.mouse.y, evt.mouse.button);
				}
				break;

			case EvtMouseWheel:
				if (m_mouse)
					m_mouse->eventMouseWheel(evt.mouse.x, evt.mouse.y, evt.mouse.delta);
				break;

			case EvtViewResize:
				if (m_stage)
					m_stage->eventResize(evt.view.width, evt.view.height);
				break;
			}
		}
		m_events.resize(0);
	}

	{
		// Finally issue the frame event.
		T_PROFILER_SCOPE(L"Flash eventFrame");
		m_movieInstance->eventFrame();
	}

	{
		// Notify frame listeners.
		T_PROFILER_SCOPE(L"Flash frame listeners");
		context->notifyFrameListeners(float(m_timeCurrent));
	}

	// Pop current movie clip.
	context->setMovieClip(current);

	{
		// Flush pool memory; release all lingering object references etc.
		T_PROFILER_SCOPE(L"Flash flush pool");
		context->getPool().flush();
	}

	// Collect reference cycles.
	if (m_gcEnable)
	{
		if (--m_framesUntilCollection <= 0)
		{
			T_PROFILER_SCOPE(L"Flash GC");
			GC::getInstance().collectCycles(false);
			m_framesUntilCollection = c_framesBetweenCollections;
		}
	}

	// Issue debugger if attached.
	if (m_movieDebugger)
		m_movieDebugger->postExecuteFrame(
			m_movie,
			m_movieInstance,
			m_stage->getFrameTransform(),
			m_stage->getViewWidth(),
			m_stage->getViewHeight()
		);
}

bool MoviePlayer::progressFrame(float deltaTime)
{
	bool executed = false;
	if (m_timeNext >= m_timeNextFrame)
	{
		m_timeCurrent = m_timeNext;
		m_timeNextFrame += 1.0f / m_movie->getMovieClip()->getFrameRate();
		executeFrame();
		executed = true;
	}
	m_timeNext += deltaTime;
	return executed;
}

void MoviePlayer::postKey(wchar_t unicode)
{
	Event evt;
	evt.eventType = EvtKey;
	evt.unicode = unicode;
	m_events.push_back(evt);
}

void MoviePlayer::postKeyDown(int32_t keyCode)
{
	Event evt;
	evt.eventType = EvtKeyDown;
	evt.keyCode = keyCode;
	m_events.push_back(evt);
}

void MoviePlayer::postKeyUp(int32_t keyCode)
{
	Event evt;
	evt.eventType = EvtKeyUp;
	evt.keyCode = keyCode;
	m_events.push_back(evt);
}

void MoviePlayer::postMouseDown(int32_t x, int32_t y, int32_t button)
{
	Vector2 xy = m_stage->toStage(Vector2(float(x), float(y)));

	Event evt;
	evt.eventType = EvtMouseDown;
	evt.mouse.x = int32_t(xy.x);
	evt.mouse.y = int32_t(xy.y);
	evt.mouse.button = button;
	m_events.push_back(evt);
}

void MoviePlayer::postMouseUp(int32_t x, int32_t y, int32_t button)
{
	Vector2 xy = m_stage->toStage(Vector2(float(x), float(y)));

	Event evt;
	evt.eventType = EvtMouseUp;
	evt.mouse.x = int32_t(xy.x);
	evt.mouse.y = int32_t(xy.y);
	evt.mouse.button = button;
	m_events.push_back(evt);
}

void MoviePlayer::postMouseMove(int32_t x, int32_t y, int32_t button)
{
	Vector2 xy = m_stage->toStage(Vector2(float(x), float(y)));

	Event evt;
	evt.eventType = EvtMouseMove;
	evt.mouse.x = int32_t(xy.x);
	evt.mouse.y = int32_t(xy.y);
	evt.mouse.button = button;
	m_events.push_back(evt);
}

void MoviePlayer::postMouseWheel(int32_t x, int32_t y, int32_t delta)
{
	Vector2 xy = m_stage->toStage(Vector2(float(x), float(y)));

	Event evt;
	evt.eventType = EvtMouseWheel;
	evt.mouse.x = int32_t(xy.x);
	evt.mouse.y = int32_t(xy.y);
	evt.mouse.delta = delta;
	m_events.push_back(evt);
}

void MoviePlayer::postViewResize(int32_t width, int32_t height)
{
	Event evt;
	evt.eventType = EvtViewResize;
	evt.view.width = width;
	evt.view.height = height;
	m_events.push_back(evt);
}

bool MoviePlayer::getFsCommand(std::string& outCommand, std::string& outArgs)
{
	if (m_fsCommands.empty())
		return false;

	outCommand = m_fsCommands.front().first;
	outArgs = m_fsCommands.front().second;

	m_fsCommands.pop_front();
	return true;
}

void MoviePlayer::setExternalCall(IExternalCall* externalCall)
{
	if (m_externalInterface)
		m_externalInterface->setExternalCall(externalCall);
}

ActionValue MoviePlayer::dispatchCallback(const std::string& methodName, int32_t argc, const ActionValue* argv)
{
	if (m_externalInterface)
		return m_externalInterface->dispatchCallback(methodName, argc, argv);
	else
		return ActionValue();
}

SpriteInstance* MoviePlayer::getMovieInstance() const
{
	return m_movieInstance;
}

const IActionVM* MoviePlayer::getVM() const
{
	return m_actionVM;
}

void MoviePlayer::setGlobal(const std::string& name, const ActionValue& value)
{
	ActionContext* actionContext = m_movieInstance->getContext();
	T_ASSERT (actionContext);

	ActionObject* global = actionContext->getGlobal();
	T_ASSERT (global);

	global->setMember(name, value);
}

ActionValue MoviePlayer::getGlobal(const std::string& name) const
{
	ActionContext* actionContext = m_movieInstance->getContext();
	T_ASSERT (actionContext);

	ActionObject* global = actionContext->getGlobal();
	T_ASSERT (global);

	ActionValue value;
	global->getMember(name, value);

	return value;
}

void MoviePlayer::setGCEnable(bool gcEnable)
{
	m_gcEnable = gcEnable;
}

void MoviePlayer::Global_getURL(CallArgs& ca)
{
	std::string url = ca.args[0].getString();
	if (startsWith< std::string >(url, "FSCommand:"))
	{
		m_fsCommands.push_back(std::make_pair(
			url.substr(10),
			ca.args[1].getString()
		));
	}
	else
		OS::getInstance().openFile(mbstows(url));
}

void MoviePlayer::Global_setInterval(CallArgs& ca)
{
	Ref< ActionObject > target;
	Ref< ActionFunction > function;
	ActionValue functionValue;
	uint32_t interval;

	if (ca.args[1].isString())
	{
		// (objectReference:Object, methodName:String, interval:Number, [param1:Object, param2, ..., paramN])
		target = ca.args[0].getObjectAlways(ca.context);
		if (!target->getMember(ca.args[1].getString(), functionValue))
			return;
		function = functionValue.getObject< ActionFunction >();
		interval = uint32_t(ca.args[2].getInteger());
	}
	else
	{
		// (functionReference:Function, interval:Number, [param1:Object, param2, ..., paramN])
		target = ca.self;
		function = ca.args[0].getObject< ActionFunction >();
		interval = uint32_t(ca.args[1].getInteger());
	}

	if (!function)
		return;

	uint32_t id = m_intervalNextId++;

	Interval& iv = m_interval[id];
	iv.count = 0;
	iv.interval = interval / m_movie->getMovieClip()->getFrameRate();
	iv.target = target;
	iv.function = function;

	ca.ret = ActionValue(int32_t(id));
}

void MoviePlayer::Global_clearInterval(CallArgs& ca)
{
	uint32_t id = uint32_t(ca.args[0].getInteger());
	std::map< uint32_t, Interval >::iterator i = m_interval.find(id);
	if (i != m_interval.end())
		m_interval.erase(i);
}

	}
}

/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Math/Const.h"
#include "Core/System/OS.h"
#include "Core/Timer/Profiler.h"
#include "Core/Timer/Timer.h"
#include "Spark/Context.h"
#include "Spark/Dictionary.h"
#include "Spark/ISoundRenderer.h"
#include "Spark/Key.h"
#include "Spark/Mouse.h"
#include "Spark/MoviePlayer.h"
#include "Spark/MovieRenderer.h"
#include "Spark/Movie.h"
#include "Spark/Sound.h"
#include "Spark/Sprite.h"
#include "Spark/SpriteInstance.h"
#include "Spark/Stage.h"
#include "Spark/Debug/MovieDebugger.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.MoviePlayer", MoviePlayer, Object)

MoviePlayer::MoviePlayer(
	const ICharacterFactory* characterFactory,
	const IMovieLoader* movieLoader,
	const MovieDebugger* movieDebugger
)
:	m_characterFactory(characterFactory)
,	m_movieLoader(movieLoader)
,	m_movieDebugger(movieDebugger)
,	m_intervalNextId(1)
,	m_timeCurrent(0.0f)
,	m_timeNext(0.0f)
,	m_timeNextFrame(0.0f)
{
}

MoviePlayer::~MoviePlayer()
{
	T_EXCEPTION_GUARD_BEGIN

	destroy();

	T_EXCEPTION_GUARD_END
}

bool MoviePlayer::create(Movie* movie, int32_t width, int32_t height, ISoundRenderer* soundRenderer)
{
	m_movie = movie;
	m_movieInstance = m_movie->createMovieClipInstance(m_characterFactory, m_movieLoader);

	const Context* context = m_movieInstance->getContext();
	m_key = context->getKey();
	m_mouse = context->getMouse();
	m_stage = context->getStage();

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
		Context* context = m_movieInstance->getContext();
		T_ASSERT(context);
		context->setFocus(nullptr);
		context->setPressed(nullptr);

		// Then destroy root movie instance.
		safeDestroy(m_movieInstance);
	}

	m_stage =  nullptr;
	m_key = nullptr;
	m_mouse = nullptr;
	m_movie = nullptr;

	m_events.clear();
}

void MoviePlayer::gotoAndPlay(uint32_t frame)
{
	m_movieInstance->gotoAndPlay(frame);
}

void MoviePlayer::gotoAndStop(uint32_t frame)
{
	m_movieInstance->gotoAndStop(frame);
}

bool MoviePlayer::gotoAndPlay(const std::string& frameLabel)
{
	return m_movieInstance->gotoAndPlay(frameLabel);
}

bool MoviePlayer::gotoAndStop(const std::string& frameLabel)
{
	return m_movieInstance->gotoAndStop(frameLabel);
}

uint32_t MoviePlayer::getFrameCount() const
{
	return m_movie->getMovieClip()->getFrameCount();
}

void MoviePlayer::render(MovieRenderer* movieRenderer) const
{
	T_PROFILER_SCOPE(L"MoviePlayer render");
	movieRenderer->render(
		m_movieInstance,
		m_movie->getFrameBounds(),
		m_stage->getFrameTransform(),
		float(m_stage->getViewWidth()),
		float(m_stage->getViewHeight())
	);
}

void MoviePlayer::execute(ISoundRenderer* soundRenderer)
{
	T_PROFILER_SCOPE(L"MoviePlayer execute");

	Context* context = m_movieInstance->getContext();
	T_ASSERT(context);

	Ref< SpriteInstance > current = context->getMovieClip();
	context->setMovieClip(m_movieInstance);

	// Issue all events in sequence as each event possibly update
	// the play head and other aspects of the movie.
	{
		T_PROFILER_SCOPE(L"MoviePlayer updateDisplayList");
		if (soundRenderer)
			m_movieInstance->updateDisplayListAndSounds(soundRenderer);
		else
			m_movieInstance->updateDisplayList();
	}

	// Dispatch events.
	if (!m_events.empty())
	{
		T_PROFILER_SCOPE(L"MoviePlayer events");
		for (const auto& event : m_events)
		{
			switch (event.eventType)
			{
			case EvtKey:
				m_movieInstance->eventKey(event.unicode);
				break;

			case EvtKeyDown:
				if (m_key)
					m_key->eventKeyDown(event.keyCode);
				m_movieInstance->eventKeyDown(event.keyCode);
				break;

			case EvtKeyUp:
				if (m_key)
					m_key->eventKeyUp(event.keyCode);
				m_movieInstance->eventKeyUp(event.keyCode);
				break;

			case EvtMouseDown:
				if (m_mouse)
					m_mouse->eventMouseDown(event.mouse.x, event.mouse.y, event.mouse.button);
				m_movieInstance->eventMouseDown(event.mouse.x, event.mouse.y, event.mouse.button);
				break;

			case EvtMouseUp:
				if (m_mouse)
					m_mouse->eventMouseUp(event.mouse.x, event.mouse.y, event.mouse.button);
				m_movieInstance->eventMouseUp(event.mouse.x, event.mouse.y, event.mouse.button);
				break;

			case EvtMouseMove:
				{
					if (m_mouse)
						m_mouse->eventMouseMove(event.mouse.x, event.mouse.y, event.mouse.button);

					// Check if mouse cursor still over "rolled over" character.
					if (context->getRolledOver())
					{
						SpriteInstance* rolledOverSprite = checked_type_cast< SpriteInstance* >(context->getRolledOver());
						bool inside = false;
						if (rolledOverSprite->isVisible())
						{
							const Matrix33 intoCharacter = rolledOverSprite->getFullTransform().inverse();
							const Aabb2 bounds = rolledOverSprite->getVisibleLocalBounds();
							const Vector2 xy = intoCharacter * Vector2(float(event.mouse.x), float(event.mouse.y));
							inside = (xy.x >= bounds.mn.x && xy.y >= bounds.mn.y && xy.x <= bounds.mx.x && xy.y <= bounds.mx.y);
						}
						if (!inside)
						{
							// Cursor has escaped; issue roll out.
							//rolledOverSprite->executeScriptEvent(ActionContext::IdOnRollOut, ActionValue());
							context->setRolledOver(nullptr);
						}
					}

					m_movieInstance->eventMouseMove(event.mouse.x, event.mouse.y, event.mouse.button);
				}
				break;

			case EvtMouseWheel:
				if (m_mouse)
					m_mouse->eventMouseWheel(event.mouse.x, event.mouse.y, event.mouse.delta);
				break;

			case EvtViewResize:
				if (m_stage)
					m_stage->eventResize(event.view.width, event.view.height);
				break;
			}
		}
		m_events.resize(0);
	}

	{
		// Finally issue the frame event.
		T_PROFILER_SCOPE(L"Spark eventFrame");
		m_movieInstance->eventFrame();
	}

	// Pop current movie clip.
	context->setMovieClip(current);

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

bool MoviePlayer::progress(float deltaTime, ISoundRenderer* soundRenderer)
{
	bool executed = false;
	if (m_timeNext >= m_timeNextFrame)
	{
		m_timeCurrent = m_timeNext;
		m_timeNextFrame += 1.0f / m_movie->getMovieClip()->getFrameRate();
		execute(soundRenderer);
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
	const Vector2 xy = m_stage->toStage(Vector2(float(x), float(y)));

	Event evt;
	evt.eventType = EvtMouseDown;
	evt.mouse.x = int32_t(xy.x);
	evt.mouse.y = int32_t(xy.y);
	evt.mouse.button = button;
	m_events.push_back(evt);
}

void MoviePlayer::postMouseUp(int32_t x, int32_t y, int32_t button)
{
	const Vector2 xy = m_stage->toStage(Vector2(float(x), float(y)));

	Event evt;
	evt.eventType = EvtMouseUp;
	evt.mouse.x = int32_t(xy.x);
	evt.mouse.y = int32_t(xy.y);
	evt.mouse.button = button;
	m_events.push_back(evt);
}

void MoviePlayer::postMouseMove(int32_t x, int32_t y, int32_t button)
{
	const Vector2 xy = m_stage->toStage(Vector2(float(x), float(y)));

	Event evt;
	evt.eventType = EvtMouseMove;
	evt.mouse.x = int32_t(xy.x);
	evt.mouse.y = int32_t(xy.y);
	evt.mouse.button = button;
	m_events.push_back(evt);
}

void MoviePlayer::postMouseWheel(int32_t x, int32_t y, int32_t delta)
{
	const Vector2 xy = m_stage->toStage(Vector2(float(x), float(y)));

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

SpriteInstance* MoviePlayer::getMovieInstance() const
{
	return m_movieInstance;
}

	}
}

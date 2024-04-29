/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::spark
{

class ICharacterFactory;
class IMovieLoader;
class ISoundRenderer;
class Key;
class Mouse;
class Movie;
class MovieDebugger;
class MovieRenderer;
class SpriteInstance;
class Stage;

/*! Spark movie player.
 * \ingroup Spark
 */
class T_DLLCLASS MoviePlayer : public Object
{
	T_RTTI_CLASS;

public:
	explicit MoviePlayer(
		const ICharacterFactory* characterFactory,
		const IMovieLoader* movieLoader,
		const MovieDebugger* movieDebugger
	);

	virtual ~MoviePlayer();

	/*! Create movie player.
	 *
	 * \param movie Root movie.
	 * \param width Output render width (in pixels).
	 * \param height Output render height (in pixels).
	 * \param soundRenderer Optional implementation of sound renderer provided to AS Sound.
	 * \return True if created successfully.
	 */
	bool create(Movie* movie, int32_t width, int32_t height, ISoundRenderer* soundRenderer);

	/*! Destroy resources used by movie player. */
	void destroy();

	/*! Goto frame and continue playing.
	 *
	 * \param frame Frame number.
	 */
	void gotoAndPlay(uint32_t frame);

	/*! Goto frame and stop playing.
	 *
	 * \param frame Frame number.
	 */
	void gotoAndStop(uint32_t frame);

	/*! Goto frame and continue playing.
	 *
	 * \param frameLabel Frame label.
	 * \return False if label not found.
	 */
	bool gotoAndPlay(const std::string& frameLabel);

	/*! Goto frame and stop playing.
	 *
	 * \param frameLabel Frame label.
	 * \return False if label not found.
	 */
	bool gotoAndStop(const std::string& frameLabel);

	/*! Get number of frames in root movie.
	 *
	 * \return Number of frames.
	 */
	uint32_t getFrameCount() const;

	/*! Render frame. */
	void render(MovieRenderer* movieRenderer) const;

	/*! Execute events in current frame. */
	void execute(ISoundRenderer* soundRenderer);

	/*! Progress until next frame.
	 *
	 * \param deltaTime Time in seconds to progress movie.
	 * \return True if new frame.
	 */
	bool progress(float deltaTime, ISoundRenderer* soundRenderer);

	/*! Post key event.
	 *
	 * \param unicode Unicode character.
	 */
	void postKey(wchar_t unicode);

	/*! Post key down event.
	 *
	 * \param keyCode Key code.
	 */
	void postKeyDown(int32_t keyCode);

	/*! Post key up event.
	 *
	 * \param keyCode Key code.
	 */
	void postKeyUp(int32_t keyCode);

	/*! Post mouse button down event.
	 *
	 * \param x Mouse cursor X coordinate.
	 * \param y Mouse cursor Y coordinate.
	 * \param button Mouse buttons being held down.
	 */
	void postMouseDown(int32_t x, int32_t y, int32_t button);

	/*! Post mouse button up event.
	 *
	 * \param x Mouse cursor X coordinate.
	 * \param y Mouse cursor Y coordinate.
	 * \param button Mouse buttons being held down.
	 */
	void postMouseUp(int32_t x, int32_t y, int32_t button);

	/*! Post mouse move event.
	 *
	 * \param x Mouse cursor X coordinate.
	 * \param y Mouse cursor Y coordinate.
	 * \param button Mouse buttons being held down.
	 */
	void postMouseMove(int32_t x, int32_t y, int32_t button);

	/*! Post mouse wheel event.
	 *
	 * \param x Mouse cursor X coordinate.
	 * \param y Mouse cursor Y coordinate.
	 * \param delta Wheel delta motion
	 */
	void postMouseWheel(int32_t x, int32_t y, int32_t delta);

	/*! Post view resize event.
	 *
	 * \param width New view width (in pixels).
	 * \param height New view height (in pixels).
	 */
	void postViewResize(int32_t width, int32_t height);

	/*! Get root movie instance.
	 *
	 * \return Root movie instance.
	 */
	SpriteInstance* getMovieInstance() const;

private:
	struct Event
	{
		uint32_t eventType;
		union
		{
			wchar_t unicode;
			int32_t keyCode;
			struct
			{
				int32_t x;
				int32_t y;
				union
				{
					int32_t button;
					int32_t delta;
				};
			}
			mouse;
			struct
			{
				int32_t width;
				int32_t height;
			}
			view;
		};
	};

	Ref< const ICharacterFactory > m_characterFactory;
	Ref< const IMovieLoader > m_movieLoader;
	Ref< const MovieDebugger > m_movieDebugger;
	Ref< Movie > m_movie;
	Ref< SpriteInstance > m_movieInstance;

	Ref< Key > m_key;
	Ref< Mouse > m_mouse;
	Ref< Stage > m_stage;

	AlignedVector< Event > m_events;
	uint32_t m_intervalNextId;
	float m_timeCurrent;
	float m_timeNext;
	float m_timeNextFrame;
};

}

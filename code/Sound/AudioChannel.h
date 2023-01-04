/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Containers/CircularVector.h"
#include "Core/Thread/DoubleBuffer.h"
#include "Sound/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

struct IFilterInstance;

class IAudioMixer;
class IFilter;
class ISoundBuffer;
class ISoundBufferCursor;

/*! Virtual audio channel.
 * \ingroup Sound
 *
 * The audio system manages a given number of virtual
 * audio channels which is responsible of managing
 * sounds attached to them and also calling appropriate
 * filters.
 */
class T_DLLCLASS AudioChannel : public Object
{
	T_RTTI_CLASS;

public:
	AudioChannel(uint32_t id, uint32_t hwSampleRate, uint32_t hwFrameSamples);

	virtual ~AudioChannel();

	/*! Set channel volume. */
	void setVolume(float volume);

	/*! Get channel volume. */
	float getVolume() const;

	/*! Set pitch. */
	void setPitch(float pitch);

	/*! Get current pitch. */
	float getPitch() const;

	/*! Associate filter in channel. */
	void setFilter(const IFilter* filter);

	/*! Set cursor parameter. */
	void setParameter(handle_t id, float parameter);

	/*! Disable repeat on current sound. */
	void disableRepeat();

	/*! Play sound through this channel.
	 *
	 * \param buffer Sound buffer.
	 * \param category Sound category.
	 * \param gain Sound gain in dB.
	 * \param repeat If sound is repeating.
	 * \param repeatFrom Skip number of samples before repeat.
	 * \return True if sound is playing successfully.
	 */
	bool play(
		const ISoundBuffer* buffer,
		handle_t category,
		float gain,
		bool repeat,
		uint32_t repeatFrom
	);

	/*! Check if there are a sound playing in this channel. */
	bool isPlaying() const;

	/*! Stop playing sound. */
	void stop();

	/*! Return current playing sound's cursor. */
	ISoundBufferCursor* getCursor();

	/*! Get next mixed and prepared sound block. */
	bool getBlock(const IAudioMixer* mixer, SoundBlock& outBlock);

private:
	friend class AudioSystem;

	struct StateFilter
	{
		Ref< const IFilter > filter;
		Ref< IFilterInstance > filterInstance;
	};

	struct StateSound
	{
		Ref< const ISoundBuffer > buffer;
		Ref< ISoundBufferCursor > cursor;
		handle_t category = 0;
		float volume = 1.0f;
		bool repeat = false;
		uint32_t repeatFrom = 0;
	};

	struct StateParameter
	{
		CircularVector< std::pair< handle_t, float >, 4 > set;
	};

	uint32_t m_id;
	uint32_t m_hwSampleRate;	//< Hardware sample rate.
	uint32_t m_hwFrameSamples;	//< Hardware frame size in samples.
	float m_volume;
	float m_pitch;
	bool m_playing;
	bool m_allowRepeat;
	DoubleBuffer< StateFilter > m_stateFilter;
	DoubleBuffer< StateSound > m_stateSound;
	DoubleBuffer< StateParameter > m_stateParameters;
	float* m_outputSamples[SbcMaxChannelCount];
	uint32_t m_outputSamplesIn;
};

}

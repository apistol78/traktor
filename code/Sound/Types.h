/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <cmath>
#include <string>
#include "Core/Config.h"
#include "Core/Platform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

/*! \ingroup Sound */
//@{

#define T_SOUND_SPEAKER_SETUP_2_0	1
#define T_SOUND_SPEAKER_SETUP_5_1	2
#define T_SOUND_SPEAKER_SETUP_7_1	3

#if defined(__IOS__) || defined(__ANDROID__)
#	define T_SOUND_SPEAKERS_SETUP T_SOUND_SPEAKER_SETUP_2_0
#else
#	define T_SOUND_SPEAKERS_SETUP T_SOUND_SPEAKER_SETUP_7_1
#endif

/*! Sound channel mapping. */
enum AudioBlockChannel
{
#if T_SOUND_SPEAKERS_SETUP == T_SOUND_SPEAKER_SETUP_2_0
	SbcLeft = 0,
	SbcRight = 1,
	SbcMaxChannelCount = 2
#elif T_SOUND_SPEAKERS_SETUP == T_SOUND_SPEAKER_SETUP_5_1
	SbcLeft = 0,
	SbcRight = 1,
	SbcCenter = 2,
	SbcLfe = 3,
	SbcRearLeft = 4,
	SbcRearRight = 5,
	SbcMaxChannelCount = 6
#elif T_SOUND_SPEAKERS_SETUP == T_SOUND_SPEAKER_SETUP_7_1
	SbcLeft = 0,
	SbcRight = 1,
	SbcCenter = 2,
	SbcLfe = 3,
	SbcRearLeft = 4,
	SbcRearRight = 5,
	SbcSideLeft = 6,
	SbcSideRight = 7,
	SbcMaxChannelCount = 8
#endif
};

/*! Parameter handle. */
typedef uint32_t handle_t;

/*! Audio driver create description. */
struct AudioDriverCreateDesc
{
	uint32_t sampleRate = 0;	//!< Playback rate, ex. 44100.
	uint8_t bitsPerSample = 0;	//!< Bits per sample, ex. 8, 16 or 24.
	uint16_t hwChannels = 0;	//!< Number of hardware channels.
	uint32_t frameSamples = 0;	//!< Size of frames, i.e. size of mixed sound blocks passed to the driver.
};

/*! Audio system create description. */
struct AudioSystemCreateDesc
{
	SystemApplication sysapp;
	uint32_t channels;									//!< Number of virtual channels.
	AudioDriverCreateDesc driverDesc;					//!< Driver create description.
	float cm[SbcMaxChannelCount][SbcMaxChannelCount];	//!< Final combine matrix.

	AudioSystemCreateDesc()
	:	channels(0)
	{
		for (int32_t i = 0; i < SbcMaxChannelCount; ++i)
			for (int32_t j = 0; j < SbcMaxChannelCount; ++j)
				cm[i][j] = (i == j) ? 1.0f : 0.0f;
	}
};

/*! Block of sound samples. */
struct AudioBlock
{
	float* samples[SbcMaxChannelCount];	//!< Point to samples, one per channel.
	uint32_t samplesCount;				//!< Number of samples.
	uint32_t sampleRate;				//!< Samples per second.
	uint32_t maxChannel;				//!< Last channel used, everyone above is considered mute.
	handle_t category;
};

/*! Return handle from parameter name.
 *
 * \param name Parameter name.
 * \return Parameter handle.
 */
handle_t T_DLLCLASS getParameterHandle(const std::wstring& name);

/*! Linear to decibel. */
inline float linearToDecibel(float lin) {
	return 20.0f * std::log10(lin);
}

/*! Decibel to linear. */
inline float decibelToLinear(float db) {
	return std::pow(10.0f, db / 20.0f);
}

//@}

}

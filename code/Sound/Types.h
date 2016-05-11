#ifndef traktor_sound_Types_H
#define traktor_sound_Types_H

#include "Core/Config.h"
#include "Core/Platform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
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

/*! \brief Static sound resource flags. */
enum StaticResourceFlags
{
	SrfZLib = 1,
	SrfLzo = 2,
	SrfLzf = 4,
	SrfDelta = 8
};

/*! \brief Sound channel mapping. */
enum SoundBlockChannel
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

/*! \brief Parameter handle. */
typedef uint32_t handle_t;

/*! \brief Sound driver create description. */
struct SoundDriverCreateDesc
{
	uint32_t sampleRate;	//!< Playback rate, ex. 44100.
	uint8_t bitsPerSample;	//!< Bits per sample, ex. 8, 16 or 24.
	uint16_t hwChannels;	//!< Number of hardware channels.
	uint32_t frameSamples;	//!< Size of frames, i.e. size of mixed sound blocks passed to the driver.

	SoundDriverCreateDesc()
	:	sampleRate(0)
	,	bitsPerSample(0)
	,	hwChannels(0)
	,	frameSamples(0)
	{
	}
};

/*! \brief Sound system create description. */
struct SoundSystemCreateDesc
{
	SystemApplication sysapp;
	uint32_t channels;									//!< Number of virtual channels.
	SoundDriverCreateDesc driverDesc;					//!< Driver create description.
	float cm[SbcMaxChannelCount][SbcMaxChannelCount];	//!< Final combine matrix.

	SoundSystemCreateDesc()
	:	channels(0)
	{
		for (int i = 0; i < SbcMaxChannelCount; ++i)
			for (int j = 0; j < SbcMaxChannelCount; ++j)
				cm[i][j] = (i == j) ? 1.0f : 0.0f;
	}
};

/*! \brief Block of sound samples. */
struct SoundBlock
{
	float* samples[SbcMaxChannelCount];	//!< Point to samples, one per channel.
	uint32_t samplesCount;				//!< Number of samples.
	uint32_t sampleRate;				//!< Samples per second.
	uint32_t maxChannel;				//!< Last channel used, everyone above is considered mute.
};

/*! \brief Block meta. */
struct SoundBlockMeta
{
	handle_t category;
	float presence;
	float presenceRate;
};

/*! \brief Return handle from parameter name.
 *
 * \param name Parameter name.
 * \return Parameter handle.
 */
handle_t T_DLLCLASS getParameterHandle(const std::wstring& name);

//@}

	}
}

#endif	// traktor_sound_Types_H

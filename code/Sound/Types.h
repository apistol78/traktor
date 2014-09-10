#ifndef traktor_sound_Types_H
#define traktor_sound_Types_H

#include "Core/Config.h"

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
	SbcLeft = 0,
	SbcRight = 1,
	SbcCenter = 2,
	SbcLfe = 3,
	SbcRearLeft = 4,
	SbcRearRight = 5,
	SbcSideLeft = 6,
	SbcSideRight = 7,
	SbcMaxChannelCount = 8
};

/*! \brief Sound driver create description. */
struct SoundDriverCreateDesc
{
	uint32_t sampleRate;	//!< Playback rate, ex. 44100.
	uint8_t bitsPerSample;	//!< Bits per sample, ex. 8, 16 or 24.
	uint16_t hwChannels;	//!< Number of hardware channels.
	uint32_t frameSamples;	//!< Size of frames, i.e. size of mixed sound blocks passed to the driver.
	uint32_t mixerFrames;	//!< Number of mixed frames ahead of submission thread.

	SoundDriverCreateDesc()
	:	sampleRate(0)
	,	bitsPerSample(0)
	,	hwChannels(0)
	,	frameSamples(0)
	,	mixerFrames(2)
	{
	}
};

/*! \brief Sound system create description. */
struct SoundSystemCreateDesc
{
	void* nativeHandle;									//!< Some kind of native handle to system.
	uint32_t channels;									//!< Number of virtual channels.
	SoundDriverCreateDesc driverDesc;					//!< Driver create description.
	float cm[SbcMaxChannelCount][SbcMaxChannelCount];	//!< Final combine matrix.

	SoundSystemCreateDesc()
	:	nativeHandle(0)
	,	channels(0)
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

/*! \brief Parameter handle. */
typedef uint32_t handle_t;

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

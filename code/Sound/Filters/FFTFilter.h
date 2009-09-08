#ifndef traktor_sound_FFTFilter_H
#define traktor_sound_FFTFilter_H

#include <vector>
#include "Sound/IFilter.h"
#include "Core/Math/Random.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

/*! \brief FFT filter.
 * \ingroup Sound
 */
class T_DLLCLASS FFTFilter : public IFilter
{
	T_RTTI_CLASS(FFTFilter)

public:
	FFTFilter(uint32_t sampleRate);

	/*!
	 * Set per frequency filter gain.
	 * \param filter Pairs of frequency (Hz) and gain.
	 * \param windowWidth Hanning window width (0 means no windowing).
	 */
	void setFilter(const std::vector< std::pair< float, float > >& filter, uint32_t windowWidth);

	virtual void apply(SoundBlock& outBlock);

private:
	enum { N = 4096 };

	uint32_t m_sampleRate;
	float m_filter[N];
	float m_history[2][N];
};

	}
}

#endif	// traktor_sound_FFTFilter_H

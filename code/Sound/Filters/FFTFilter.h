#ifndef traktor_sound_FFTFilter_H
#define traktor_sound_FFTFilter_H

#include <vector>
#include "Sound/IFilter.h"
#include "Core/Math/Random.h"

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

/*! \brief FFT filter.
 * \ingroup Sound
 */
class T_DLLCLASS FFTFilter : public IFilter
{
	T_RTTI_CLASS;

public:
	enum { N = 4096 };

	FFTFilter(uint32_t sampleRate = 44100);

	/*!
	 * Set per frequency filter gain.
	 * \param filter Pairs of frequency (Hz) and gain.
	 * \param windowWidth Hanning window width (0 means no windowing).
	 */
	void setFilter(const std::vector< std::pair< float, float > >& filter, uint32_t windowWidth);

	virtual Ref< IFilterInstance > createInstance() const;

	virtual void apply(IFilterInstance* instance, SoundBlock& outBlock) const;

	virtual void serialize(ISerializer& s);

private:
	uint32_t m_sampleRate;
	float m_filter[N];
};

	}
}

#endif	// traktor_sound_FFTFilter_H

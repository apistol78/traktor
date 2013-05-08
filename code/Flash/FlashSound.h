#ifndef traktor_flash_FlashSound_H
#define traktor_flash_FlashSound_H

#include "Core/Misc/AutoPtr.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

/*! \brief Flash sound container.
 * \ingroup Flash
 */
class T_DLLCLASS FlashSound : public ISerializable
{
	T_RTTI_CLASS;

public:
	FlashSound();

	bool create(uint8_t channels, uint32_t sampleRate, uint32_t samplesCount);

	uint8_t getChannels() const { return m_channels; }

	uint32_t getSampleRate() const { return m_sampleRate; }

	uint32_t getSamplesCount() const { return m_samplesCount; }

	int16_t* getSamples(uint8_t channel) { return m_samples[channel].ptr(); }

	const int16_t* getSamples(uint8_t channel) const { return m_samples[channel].c_ptr(); }

	virtual void serialize(ISerializer& s);

private:
	uint8_t m_channels;
	uint32_t m_sampleRate;
	uint32_t m_samplesCount;
	AutoArrayPtr< int16_t > m_samples[2];
};

	}
}

#endif	// traktor_flash_FlashSound_H

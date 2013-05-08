#ifndef traktor_sound_NormalizationFilter_H
#define traktor_sound_NormalizationFilter_H

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

/*! \brief Normalization filter.
 * \ingroup Sound
 */
class T_DLLCLASS NormalizationFilter : public IFilter
{
	T_RTTI_CLASS;

public:
	NormalizationFilter(float targetEnergy = 1.0f, float energyThreshold = 0.2f, float attackRate = 0.1f);

	virtual Ref< IFilterInstance > createInstance() const;

	virtual void apply(IFilterInstance* instance, SoundBlock& outBlock) const;

	virtual void serialize(ISerializer& s);

	inline float getTargetEnergy() const { return m_targetEnergy; }

	inline float getEnergyThreshold() const { return m_energyThreshold; }

	inline float getAttackRate() const { return m_attackRate; }

private:
	float m_targetEnergy;
	float m_energyThreshold;
	float m_attackRate;
};

	}
}

#endif	// traktor_sound_NormalizationFilter_H

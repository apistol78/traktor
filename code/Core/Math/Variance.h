#pragma once

#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Measure variance of input signal.
 * \ingroup Core
 */
class T_DLLCLASS Variance
{
public:
	Variance();

	void insert(float value);

    bool empty() const;

    float getMean() const;

	float getVariance() const;

	bool stop(float accept, float confidence) const;

private:
	int32_t m_count;
	float m_mean;
	float m_meanDistSquared;
};

}

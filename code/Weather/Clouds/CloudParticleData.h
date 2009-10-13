#ifndef traktor_weather_CloudParticleData_H
#define traktor_weather_CloudParticleData_H

#include "Core/Serialization/Serializable.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEATHER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace weather
	{

class T_DLLCLASS CloudParticleData : public Serializable
{
	T_RTTI_CLASS(CloudParticleData)

public:
	CloudParticleData();

	virtual bool serialize(Serializer& s);

	uint32_t getCount() const { return m_count; }

	float getRadiusMin() const { return m_radiusMin; }

	float getRadiusRange() const { return m_radiusRange; }

	float getDensity() const { return m_density; }

	const Vector4& getSize() const { return m_size; }

	int getOctaves() const { return m_octaves; }

	float getPersistence() const { return m_persistence; }

	float getNoiseScale() const { return m_noiseScale; }

private:
	uint32_t m_count;
	float m_radiusMin;
	float m_radiusRange;
	float m_density;
	Vector4 m_size;
	int32_t m_octaves;
	float m_persistence;
	float m_noiseScale;
};

	}
}

#endif	// traktor_weather_CloudParticleData_H

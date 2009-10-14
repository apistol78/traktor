#ifndef traktor_weather_CloudParticleCluster_H
#define traktor_weather_CloudParticleCluster_H

#include "Core/Math/Aabb.h"
#include "Core/Containers/AlignedVector.h"
#include "Weather/Clouds/CloudParticle.h"

namespace traktor
{
	namespace weather
	{

class CloudParticleData;

class CloudParticleCluster
{
public:
	bool create(const CloudParticleData& particleData);

	void update(const CloudParticleData& particleData, float deltaTime);

	const Aabb& getBoundingBox() const;

	const AlignedVector< CloudParticle >& getParticles() const;

private:
	Aabb m_boundingBox;
	AlignedVector< CloudParticle > m_particles;
};

	}
}

#endif	// traktor_weather_CloudParticleCluster_H

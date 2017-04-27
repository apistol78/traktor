/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_weather_CloudParticleCluster_H
#define traktor_weather_CloudParticleCluster_H

#include "Core/Math/Aabb3.h"
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

	const Aabb3& getBoundingBox() const;

	const AlignedVector< CloudParticle >& getParticles() const;

private:
	Aabb3 m_boundingBox;
	AlignedVector< CloudParticle > m_particles;
};

	}
}

#endif	// traktor_weather_CloudParticleCluster_H

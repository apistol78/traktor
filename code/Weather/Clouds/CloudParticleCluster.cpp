/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Weather/Clouds/CloudParticleCluster.h"
#include "Weather/Clouds/CloudParticleData.h"
#include "Core/Math/Const.h"
#include "Core/Math/Random.h"

namespace traktor
{
	namespace weather
	{
		namespace
		{

float noise(int x, int y)
{
	x = x + y * 57;
	x = (x << 13) ^ x;
	return (1.0f - ((x * (x * x * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

float smoothNoise(int x, int y)
{
	float c = (noise(x - 1, y - 1) + noise(x + 1, y - 1) + noise(x - 1, y + 1) + noise(x + 1, y + 1)) / 16;
	float s = (noise(x - 1, y) + noise(x + 1, y) + noise(x, y - 1) + noise(x, y + 1)) / 8;
	float e = noise(x, y) / 4;
	return c + s + e;
}

float interpolate(float a, float b, float t)
{
	float ft = float(t * PI);
	float f = float(1.0f - std::cos(ft)) * 0.5f;
	return a * (1 - f) + b * f;
}

float interpolateNoise(float x, float y)
{
	int ix = int(x);
	float fx = x - ix;
	
	int iy = int(y);
	float fy = y - iy;

	float v[] =
	{
		smoothNoise(ix, iy),
		smoothNoise(ix + 1, iy),
		smoothNoise(ix, iy + 1),
		smoothNoise(ix + 1, iy + 1)
	};

	float i[] =
	{
		interpolate(v[0], v[1], fx),
		interpolate(v[2], v[3], fx)
	};

	return interpolate(i[0], i[1], fy);
}

		}

bool CloudParticleCluster::create(const CloudParticleData& particleData)
{
	Random random;

	m_particles.resize(particleData.getCount());
	for (uint32_t i = 0; i < m_particles.size(); ++i)
	{
		float x = float(random.nextDouble() * 2.0 - 1.0);
		float z = float(random.nextDouble() * 2.0 - 1.0);

		float y = 0.0f;
		for (int j = 0; j < particleData.getOctaves(); ++j)
		{
			float frequency = std::pow(2.0f, j);
			float amplitude = std::pow(particleData.getPersistence(), j);
			y += interpolateNoise(x * frequency * particleData.getNoiseScale(), z * frequency * particleData.getNoiseScale()) * amplitude;
		}

		m_particles[i].position = (Vector4(x, y, z, 0.0f) * particleData.getSize()).xyz1();
		m_particles[i].positionVelocity = float(random.nextDouble() * 0.2f) + 0.05f;
		m_particles[i].maxRadius = float(particleData.getRadiusMin() + particleData.getRadiusRange() * random.nextDouble());
		m_particles[i].radius = m_particles[i].maxRadius;
		m_particles[i].rotation = float(random.nextDouble() * 2.0 - 1.0) * PI;
		m_particles[i].rotationVelocity = float(random.nextDouble() * 2.0 - 1.0) * 0.2f;
		m_particles[i].sprite = int(random.nextDouble() * 4.0);
	}

	Scalar radiusExtent = Scalar(2.0f * (particleData.getRadiusMin() + particleData.getRadiusRange()));
	m_boundingBox.mn = -particleData.getSize().xyz0() * Scalar(0.5f) - radiusExtent;
	m_boundingBox.mx =  particleData.getSize().xyz0() * Scalar(0.5f) + radiusExtent;

	return true;
}

void CloudParticleCluster::update(const CloudParticleData& particleData, float deltaTime)
{
	for (AlignedVector< CloudParticle >::iterator i = m_particles.begin(); i != m_particles.end(); ++i)
	{
		i->position += Vector4(1.0f, 0.0f, 0.0f, 0.0f) * Scalar(deltaTime * i->positionVelocity);
		if (i->position.x() >= particleData.getSize().x())
			i->position.set(0, -particleData.getSize().x());

		i->rotation += i->rotationVelocity * deltaTime;
	}
}

const Aabb3& CloudParticleCluster::getBoundingBox() const
{
	return m_boundingBox;
}

const AlignedVector< CloudParticle >& CloudParticleCluster::getParticles() const
{
	return m_particles;
}

	}
}
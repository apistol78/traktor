/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_weather_CloudParticleData_H
#define traktor_weather_CloudParticleData_H

#include "Core/Math/Color4ub.h"
#include "Core/Math/Vector4.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEATHER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace weather
	{

class T_DLLCLASS CloudParticleData : public ISerializable
{
	T_RTTI_CLASS;

public:
	CloudParticleData();

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	uint32_t getCount() const { return m_count; }

	float getRadiusMin() const { return m_radiusMin; }

	float getRadiusRange() const { return m_radiusRange; }

	float getDensity() const { return m_density; }

	const Color4ub& getHaloColor() const { return m_haloColor; }

	const Color4ub& getSkyColor() const { return m_skyColor; }

	const Color4ub& getGroundColor() const { return m_groundColor; }

	float getSunInfluence() const { return m_sunInfluence; }

	const Vector4& getSize() const { return m_size; }

	int getOctaves() const { return m_octaves; }

	float getPersistence() const { return m_persistence; }

	float getNoiseScale() const { return m_noiseScale; }

private:
	uint32_t m_count;
	float m_radiusMin;
	float m_radiusRange;
	float m_density;
	Color4ub m_haloColor;
	Color4ub m_skyColor;
	Color4ub m_groundColor;
	float m_sunInfluence;
	Vector4 m_size;
	int32_t m_octaves;
	float m_persistence;
	float m_noiseScale;
};

	}
}

#endif	// traktor_weather_CloudParticleData_H

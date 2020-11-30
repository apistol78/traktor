#pragma once

#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace shape
	{

class T_DLLCLASS BakeConfiguration : public ISerializable
{
	T_RTTI_CLASS;

public:
	uint32_t getSampleCount() const { return m_sampleCount; }

	uint32_t getShadowSampleCount() const { return m_shadowSampleCount; }

	uint32_t getIrradianceSampleCount() const { return m_irradianceSampleCount; }

	float getMaxPathDistance() const { return m_maxPathDistance; }

	float getPointLightShadowRadius() const { return m_pointLightShadowRadius; }

	float getLumelDensity() const { return m_lumelDensity; }

	float getIrradianceGridDensity() const { return m_irradianceGridDensity; }

	int32_t getMinimumLightMapSize() const { return m_minimumLightMapSize; }

	int32_t getMaximumLightMapSize() const { return m_maximumLightMapSize; }

	bool getEnableDenoise() const { return m_enableDenoise; }

	bool getEnableSeamFilter() const { return m_enableSeamFilter; }

	virtual void serialize(ISerializer& s) override final;

private:
	uint32_t m_sampleCount = 100;
	uint32_t m_shadowSampleCount = 50;
	uint32_t m_irradianceSampleCount = 50;
	float m_maxPathDistance = 1.0f;
	float m_pointLightShadowRadius = 0.1f;
	float m_lumelDensity = 16.0f;
	float m_irradianceGridDensity = 1.0f;
	int32_t m_minimumLightMapSize = 16;
	int32_t m_maximumLightMapSize = 1024;
	bool m_enableDenoise = true;
	bool m_enableSeamFilter = true;
};

	}
}
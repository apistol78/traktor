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
	BakeConfiguration();

	const Guid& getSeedGuid() const { return m_seedGuid; }

	bool traceDirect() const { return m_traceDirect; }

	bool traceIndirect() const { return m_traceIndirect; }

	bool traceIrradiance() const { return m_traceIrradiance; }

	uint32_t getIndirectSampleCount() const { return m_indirectSampleCount; }

	uint32_t getShadowSampleCount() const { return m_shadowSampleCount; }

	uint32_t getIrradianceSampleCount() const { return m_irradianceSampleCount; }

	float getPointLightShadowRadius() const { return m_pointLightShadowRadius; }

	float getLumelDensity() const { return m_lumelDensity; }

	int32_t getMinimumLightMapSize() const { return m_minimumLightMapSize; }

	bool getEnableShadowFix() const { return m_enableShadowFix; }

	bool getEnableDenoise() const { return m_enableDenoise; }

	float getClampShadowThreshold() const { return m_clampShadowThreshold; }

	virtual void serialize(ISerializer& s) override final;

private:
	Guid m_seedGuid;
	bool m_traceDirect;
	bool m_traceIndirect;
	bool m_traceIrradiance;
	uint32_t m_indirectSampleCount;
	uint32_t m_shadowSampleCount;
	uint32_t m_irradianceSampleCount;
	float m_pointLightShadowRadius;
	float m_lumelDensity;
	int32_t m_minimumLightMapSize;
	bool m_enableShadowFix;
	bool m_enableDenoise;
	float m_clampShadowThreshold;
};

	}
}
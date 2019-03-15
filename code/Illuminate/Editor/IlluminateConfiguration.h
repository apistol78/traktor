#pragma once

#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ILLUMINATE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace illuminate
	{

class T_DLLCLASS IlluminateConfiguration : public ISerializable
{
	T_RTTI_CLASS;

public:
	IlluminateConfiguration();

	const Guid& getSeedGuid() const { return m_seedGuid; }

	bool traceDirect() const { return m_traceDirect; }

	bool traceIndirect() const { return m_traceIndirect; }

	bool traceOcclusion() const { return m_traceOcclusion; }

	uint32_t getIrradianceSampleCount() const { return m_irradianceSampleCount; }

	uint32_t getShadowSampleCount() const { return m_shadowSampleCount; }

	uint32_t getOcclusionSampleCount() const { return m_occlusionSampleCount; }

	float getPointLightShadowRadius() const { return m_pointLightShadowRadius; }

	float getLumelDensity() const { return m_lumelDensity; }

	uint32_t getPhotonCount() const { return m_photonCount; }

	float getPhotonSampleRadius() const { return m_photonSampleRadius; }

	virtual void serialize(ISerializer& s) override final;

private:
	Guid m_seedGuid;
	bool m_traceDirect;
	bool m_traceIndirect;
	bool m_traceOcclusion;
	uint32_t m_irradianceSampleCount;
	uint32_t m_shadowSampleCount;
	uint32_t m_occlusionSampleCount;
	float m_pointLightShadowRadius;
	float m_lumelDensity;
	uint32_t m_photonCount;
	float m_photonSampleRadius;
};

	}
}
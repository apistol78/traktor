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

	uint32_t getIndirectSampleCount() const { return m_indirectSampleCount; }

	uint32_t getShadowSampleCount() const { return m_shadowSampleCount; }

	float getPointLightShadowRadius() const { return m_pointLightShadowRadius; }

	float getLumelDensity() const { return m_lumelDensity; }

	virtual void serialize(ISerializer& s) override final;

private:
	Guid m_seedGuid;
	bool m_traceDirect;
	bool m_traceIndirect;
	uint32_t m_indirectSampleCount;
	uint32_t m_shadowSampleCount;
	float m_pointLightShadowRadius;
	float m_lumelDensity;
};

	}
}
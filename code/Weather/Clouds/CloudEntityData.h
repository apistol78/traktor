#ifndef traktor_weather_CloudEntityData_H
#define traktor_weather_CloudEntityData_H

#include "World/Entity/SpatialEntityData.h"
#include "Resource/Proxy.h"
#include "Weather/Clouds/CloudParticleData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEATHER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace render
	{

class IRenderSystem;
class Shader;

	}

	namespace weather
	{

class CloudEntity;

class T_DLLCLASS CloudEntityData : public world::SpatialEntityData
{
	T_RTTI_CLASS(CloudEntityData)

public:
	CloudEntityData();

	CloudEntity* createEntity(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const;

	virtual bool serialize(Serializer& s);

	inline const resource::Proxy< render::Shader >& getParticleShader() const { return m_particleShader; }

	inline const resource::Proxy< render::Shader >& getImpostorShader() const { return m_impostorShader; }

private:
	mutable resource::Proxy< render::Shader > m_particleShader;
	mutable resource::Proxy< render::Shader > m_impostorShader;
	uint32_t m_impostorTargetResolution;
	uint32_t m_impostorSliceCount;
	float m_impostorSliceOffset;
	CloudParticleData m_particleData;
};

	}
}

#endif	// traktor_weather_CloudEntityData_H

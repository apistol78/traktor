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
	namespace render
	{

class RenderSystem;
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

	CloudEntity* createEntity(render::RenderSystem* renderSystem) const;

	virtual bool serialize(Serializer& s);

private:
	resource::Proxy< render::Shader > m_particleShader;
	resource::Proxy< render::Shader > m_impostorShader;
	uint32_t m_impostorTargetResolution;
	uint32_t m_distanceTargetResolution;
	CloudParticleData m_particleData;
};

	}
}

#endif	// traktor_weather_CloudEntityData_H

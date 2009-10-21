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
class ITexture;
class Shader;

	}

	namespace weather
	{

class CloudEntity;
class CloudMask;

class T_DLLCLASS CloudEntityData : public world::SpatialEntityData
{
	T_RTTI_CLASS(CloudEntityData)

public:
	CloudEntityData();

	CloudEntity* createEntity(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const;

	virtual int getVersion() const;

	virtual bool serialize(Serializer& s);

	inline const resource::Proxy< render::Shader >& getParticleShader() const { return m_particleShader; }

	inline const resource::Proxy< render::ITexture >& getParticleTexture() const { return m_particleTexture; }

	inline const resource::Proxy< render::Shader >& getImpostorShader() const { return m_impostorShader; }

	inline const resource::Proxy< CloudMask >& getMask() const { return m_mask; }

private:
	mutable resource::Proxy< render::Shader > m_particleShader;
	mutable resource::Proxy< render::ITexture > m_particleTexture;
	mutable resource::Proxy< render::Shader > m_impostorShader;
	mutable resource::Proxy< CloudMask > m_mask;
	uint32_t m_impostorTargetResolution;
	uint32_t m_impostorSliceCount;
	uint32_t m_updateFrequency;
	float m_updatePositionThreshold;
	float m_updateDirectionThreshold;
	CloudParticleData m_particleData;
};

	}
}

#endif	// traktor_weather_CloudEntityData_H

#ifndef traktor_weather_CloudEntityData_H
#define traktor_weather_CloudEntityData_H

#include "Resource/Id.h"
#include "Weather/Clouds/CloudParticleData.h"
#include "World/EntityData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEATHER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
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

class T_DLLCLASS CloudEntityData : public world::EntityData
{
	T_RTTI_CLASS;

public:
	CloudEntityData();

	Ref< CloudEntity > createEntity(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const;

	virtual void serialize(ISerializer& s);

	const resource::Id< render::Shader >& getParticleShader() const { return m_particleShader; }

	const resource::Id< render::ITexture >& getParticleTexture() const { return m_particleTexture; }

	const resource::Id< render::Shader >& getImpostorShader() const { return m_impostorShader; }

	const resource::Id< CloudMask >& getMask() const { return m_mask; }

private:
	resource::Id< render::Shader > m_particleShader;
	resource::Id< render::ITexture > m_particleTexture;
	resource::Id< render::Shader > m_impostorShader;
	resource::Id< CloudMask > m_mask;
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

#ifndef traktor_weather_SkyEntityData_H
#define traktor_weather_SkyEntityData_H

#include "World/Entity/EntityData.h"
#include "Resource/Proxy.h"

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

class SkyEntity;

class T_DLLCLASS SkyEntityData : public world::EntityData
{
	T_RTTI_CLASS(SkyEntityData)

public:
	SkyEntityData();

	Ref< SkyEntity > createEntity(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const;

	virtual bool serialize(Serializer& s);

	inline const resource::Proxy< render::Shader >& getShader() const { return m_shader; }

private:
	mutable resource::Proxy< render::Shader > m_shader;
};

	}
}

#endif	// traktor_weather_SkyEntityData_H

#ifndef traktor_weather_SkyEntityData_H
#define traktor_weather_SkyEntityData_H

#include "Core/Math/Vector4.h"
#include "Resource/Id.h"
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
class Shader;

	}

	namespace weather
	{

class SkyEntity;

class T_DLLCLASS SkyEntityData : public world::EntityData
{
	T_RTTI_CLASS;

public:
	SkyEntityData();

	Ref< SkyEntity > createEntity(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const;

	virtual void serialize(ISerializer& s);

	const resource::Id< render::Shader >& getShader() const { return m_shader; }

private:
	resource::Id< render::Shader > m_shader;
	Vector4 m_sunDirection;
	float m_offset;
};

	}
}

#endif	// traktor_weather_SkyEntityData_H

#ifndef traktor_weather_PrecipitationComponentData_H
#define traktor_weather_PrecipitationComponentData_H

#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEATHER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IRenderSystem;
class Shader;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace weather
	{

class PrecipitationComponent;

/*! \brief Precipitation component data.
 * \ingroup Weather
 */
class T_DLLCLASS PrecipitationComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	PrecipitationComponentData();

	Ref< PrecipitationComponent > createComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const resource::Id< render::Shader >& getShader() const { return m_shader; }

private:
	resource::Id< render::Shader > m_shader;
	uint32_t m_layers;
	float m_distance;
};

	}
}

#endif	// traktor_weather_PrecipitationComponentData_H

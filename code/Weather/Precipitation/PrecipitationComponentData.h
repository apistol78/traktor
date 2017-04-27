/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
	namespace mesh
	{

class StaticMesh;

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

	Ref< PrecipitationComponent > createComponent(resource::IResourceManager* resourceManager) const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const resource::Id< mesh::StaticMesh >& getMesh() const { return m_mesh; }

private:
	resource::Id< mesh::StaticMesh > m_mesh;
	float m_tiltRate;
	float m_parallaxDistance;
	float m_depthDistance;
	float m_opacity;
};

	}
}

#endif	// traktor_weather_PrecipitationComponentData_H

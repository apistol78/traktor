#ifndef traktor_amalgam_WorldLayerData_H
#define traktor_amalgam_WorldLayerData_H

#include <map>
#include "Amalgam/Engine/LayerData.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace scene
	{

class Scene;

	}

	namespace world
	{

class EntityData;

	}

	namespace amalgam
	{

class T_DLLCLASS WorldLayerData : public LayerData
{
	T_RTTI_CLASS;

public:
	virtual Ref< Layer > createInstance(Stage* stage, amalgam::IEnvironment* environment) const;

	virtual void serialize(ISerializer& s);

private:
	friend class StagePipeline;

	resource::Id< scene::Scene > m_scene;
	std::map< std::wstring, resource::Id< world::EntityData > > m_entities;
};

	}
}

#endif	// traktor_amalgam_WorldLayerData_H

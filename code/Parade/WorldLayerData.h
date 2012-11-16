#ifndef traktor_parade_WorldLayerData_H
#define traktor_parade_WorldLayerData_H

#include <map>
#include "Resource/Id.h"
#include "Parade/LayerData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PARADE_EXPORT)
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

	namespace parade
	{

class T_DLLCLASS WorldLayerData : public LayerData
{
	T_RTTI_CLASS;

public:
	virtual Ref< Layer > createInstance(Stage* stage, amalgam::IEnvironment* environment) const;

	virtual bool serialize(ISerializer& s);

private:
	friend class StagePipeline;

	resource::Id< scene::Scene > m_scene;
	std::map< std::wstring, resource::Id< world::EntityData > > m_entities;
};

	}
}

#endif	// traktor_parade_WorldLayerData_H

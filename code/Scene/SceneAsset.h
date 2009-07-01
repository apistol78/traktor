#ifndef traktor_scene_SceneAsset_H
#define traktor_scene_SceneAsset_H

#include "Core/Heap/Ref.h"
#include "Core/Serialization/Serializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class WorldRenderSettings;
class EntityInstance;

	}

	namespace scene
	{

class T_DLLCLASS SceneAsset : public Serializable
{
	T_RTTI_CLASS(SceneAsset)

public:
	SceneAsset();

	void setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings);

	world::WorldRenderSettings* getWorldRenderSettings() const;

	void setInstance(world::EntityInstance* instance);

	world::EntityInstance* getInstance() const;

	virtual bool serialize(Serializer& s);

private:
	Ref< world::WorldRenderSettings > m_worldRenderSettings;
	Ref< world::EntityInstance > m_instance;
};

	}
}

#endif	// traktor_scene_SceneAsset_H

#ifndef traktor_scene_SceneAsset_H
#define traktor_scene_SceneAsset_H

#include "Editor/ITypedAsset.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class EntityData;
class PostProcessSettings;
class WorldRenderSettings;

	}

	namespace scene
	{

class ISceneControllerData;

class T_DLLCLASS SceneAsset : public editor::ITypedAsset
{
	T_RTTI_CLASS;

public:
	SceneAsset();

	void setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings);

	Ref< world::WorldRenderSettings > getWorldRenderSettings() const;

	void setPostProcessSettings(const resource::Proxy< world::PostProcessSettings >& postProcess);

	const resource::Proxy< world::PostProcessSettings >& getPostProcessSettings() const;

	void setEntityData(world::EntityData* entityData);

	Ref< world::EntityData > getEntityData() const;

	void setControllerData(ISceneControllerData* controllerData);

	Ref< ISceneControllerData > getControllerData() const;

	virtual const TypeInfo* getOutputType() const;

	virtual bool serialize(ISerializer& s);

private:
	Ref< world::WorldRenderSettings > m_worldRenderSettings;
	mutable resource::Proxy< world::PostProcessSettings > m_postProcessSettings;
	Ref< world::EntityData > m_entityData;
	Ref< ISceneControllerData > m_controllerData;
};

	}
}

#endif	// traktor_scene_SceneAsset_H

#ifndef traktor_scene_SceneAsset_H
#define traktor_scene_SceneAsset_H

#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

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

class PostProcessSettings;
class WorldRenderSettings;

	}

	namespace scene
	{

class ISceneControllerData;
class LayerEntityData;

class T_DLLCLASS SceneAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	SceneAsset();

	void setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings);

	Ref< world::WorldRenderSettings > getWorldRenderSettings() const;

	void setPostProcessSettings(const resource::Id< world::PostProcessSettings >& postProcess);

	const resource::Id< world::PostProcessSettings >& getPostProcessSettings() const;

	void setLayers(const RefArray< LayerEntityData >& layers);

	const RefArray< LayerEntityData >& getLayers() const;

	void setControllerData(ISceneControllerData* controllerData);

	Ref< ISceneControllerData > getControllerData() const;

	virtual bool serialize(ISerializer& s);

private:
	Ref< world::WorldRenderSettings > m_worldRenderSettings;
	resource::Id< world::PostProcessSettings > m_postProcessSettings;
	RefArray< LayerEntityData > m_layers;
	Ref< ISceneControllerData > m_controllerData;
};

	}
}

#endif	// traktor_scene_SceneAsset_H

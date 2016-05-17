#ifndef traktor_scene_SceneAsset_H
#define traktor_scene_SceneAsset_H

#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"
#include "World/WorldTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ImageProcessSettings;
class ITexture;

	}

	namespace world
	{

class LayerEntityData;
class WorldRenderSettings;

	}

	namespace scene
	{

class ISceneControllerData;

class T_DLLCLASS SceneAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	SceneAsset();

	void setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings);

	Ref< world::WorldRenderSettings > getWorldRenderSettings() const;

	void setImageProcessParams(const SmallMap< std::wstring, resource::Id< render::ITexture > >& imageProcessParams);

	const SmallMap< std::wstring, resource::Id< render::ITexture > >& getImageProcessParams() const;

	void setLayers(const RefArray< world::LayerEntityData >& layers);

	const RefArray< world::LayerEntityData >& getLayers() const;

	void setControllerData(ISceneControllerData* controllerData);

	Ref< ISceneControllerData > getControllerData() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	Ref< world::WorldRenderSettings > m_worldRenderSettings;
	SmallMap< std::wstring, resource::Id< render::ITexture > > m_imageProcessParams;
	RefArray< world::LayerEntityData > m_layers;
	Ref< ISceneControllerData > m_controllerData;
};

	}
}

#endif	// traktor_scene_SceneAsset_H

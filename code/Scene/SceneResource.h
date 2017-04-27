/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_scene_SceneResource_H
#define traktor_scene_SceneResource_H

#include "Core/Containers/SmallMap.h"
#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"
#include "World/WorldRenderSettings.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EXPORT)
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

class ImageProcessSettings;
class IRenderSystem;
class ITexture;

	}

	namespace world
	{

class IEntityBuilder;
class EntityData;
class WorldRenderSettings;

	}

	namespace scene
	{

class ISceneControllerData;
class Scene;

/*! \brief Scene resource.
 * \ingroup Scene
 */
class T_DLLCLASS SceneResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	SceneResource();

	Ref< Scene > createScene(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		world::IEntityBuilder* entityBuilder
	) const;

	void setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings);

	Ref< world::WorldRenderSettings > getWorldRenderSettings() const;

	void setImageProcessParams(const SmallMap< std::wstring, resource::Id< render::ITexture > >& imageProcessParams);

	const SmallMap< std::wstring, resource::Id< render::ITexture > >& getImageProcessParams() const;

	void setEntityData(world::EntityData* entityData);

	Ref< world::EntityData > getEntityData() const;

	void setControllerData(ISceneControllerData* controllerData);

	Ref< ISceneControllerData > getControllerData() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	Ref< world::WorldRenderSettings > m_worldRenderSettings;
	SmallMap< std::wstring, resource::Id< render::ITexture > > m_imageProcessParams;
	Ref< world::EntityData > m_entityData;
	Ref< ISceneControllerData > m_controllerData;
};

	}
}

#endif	// traktor_scene_SceneResource_H

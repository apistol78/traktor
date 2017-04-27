/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_scene_ScenePermutationAsset_H
#define traktor_scene_ScenePermutationAsset_H

#include <list>
#include "Core/Containers/SmallMap.h"
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

class WorldRenderSettings;

	}

	namespace scene
	{

class T_DLLCLASS ScenePermutationAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	friend class ScenePermutationPipeline;

	Guid m_scene;
	std::list< std::wstring > m_includeLayers;
	Ref< world::WorldRenderSettings > m_overrideWorldRenderSettings;
	SmallMap< std::wstring, resource::Id< render::ITexture > > m_overrideImageProcessParams;
};

	}
}

#endif	// traktor_scene_ScenePermutationAsset_H

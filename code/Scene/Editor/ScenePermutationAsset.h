#pragma once

#include <list>
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
	virtual void serialize(ISerializer& s) override final;

private:
	friend class ScenePermutationPipeline;

	Guid m_scene;
	std::list< std::wstring > m_includeLayers;
	Ref< world::WorldRenderSettings > m_overrideWorldRenderSettings;
};

	}
}


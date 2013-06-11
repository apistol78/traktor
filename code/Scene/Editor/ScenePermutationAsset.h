#ifndef traktor_scene_ScenePermutationAsset_H
#define traktor_scene_ScenePermutationAsset_H

#include <list>
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace scene
	{

class T_DLLCLASS ScenePermutationAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s);

private:
	friend class ScenePermutationPipeline;

	Guid m_scene;
	std::list< std::wstring > m_includeLayers;
};

	}
}

#endif	// traktor_scene_ScenePermutationAsset_H

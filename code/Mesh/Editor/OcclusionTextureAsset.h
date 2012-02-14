#ifndef traktor_mesh_OcclusionTextureAsset_H
#define traktor_mesh_OcclusionTextureAsset_H

#include "Render/Editor/Texture/TextureAssetBase.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class T_DLLCLASS OcclusionTextureAsset : public render::TextureAssetBase
{
	T_RTTI_CLASS;

public:
	virtual Ref< drawing::Image > load(const std::wstring& assetPath) const;
};

	}
}

#endif	// traktor_mesh_OcclusionTextureAsset_H

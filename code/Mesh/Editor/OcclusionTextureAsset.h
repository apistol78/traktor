#ifndef traktor_mesh_OcclusionTextureAsset_H
#define traktor_mesh_OcclusionTextureAsset_H

#include "Editor/Asset.h"
#include "Render/Editor/Texture/TextureOutput.h"

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

class T_DLLCLASS OcclusionTextureAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	render::TextureOutput m_output;

	virtual void serialize(ISerializer& s);
};

	}
}

#endif	// traktor_mesh_OcclusionTextureAsset_H

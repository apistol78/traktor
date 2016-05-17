#ifndef traktor_render_TextureAsset_H
#define traktor_render_TextureAsset_H

#include "Editor/Asset.h"
#include "Render/Editor/Texture/TextureOutput.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class T_DLLCLASS TextureAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	TextureOutput m_output;

	virtual const TypeInfo* getOutputType() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_render_TextureAsset_H

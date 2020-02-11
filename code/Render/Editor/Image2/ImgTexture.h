#pragma once

#include "Render/Editor/ImmutableNode.h"
#include "Resource/Id.h"

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

class ITexture;

class T_DLLCLASS ImgTexture : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	ImgTexture();

	const resource::Id< ITexture >& getTexture() const;

	virtual void serialize(ISerializer& s) override final;

private:
    friend class ImageGraphPipeline;

	resource::Id< ITexture > m_texture;
};

	}   
}
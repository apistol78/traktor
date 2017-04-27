/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_TextureLinker_H
#define traktor_render_TextureLinker_H

#include "Render/Resource/ShaderResource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ITexture;

/*! \brief Bind textures to shaders.
 * \ingroup Render
 */
class T_DLLCLASS TextureLinker : public Object
{
	T_RTTI_CLASS;

public:
	struct TextureReader
	{
		virtual ~TextureReader() {}

		virtual Ref< ITexture > read(const Guid& textureGuid) = 0;
	};

	TextureLinker(TextureReader& textureReader);

	bool link(const ShaderResource::Combination& shaderCombination, IProgram* program);

private:
	TextureReader& m_textureReader;
};

	}
}

#endif	// traktor_render_TextureLinker_H

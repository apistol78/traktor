/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/IProgram.h"
#include "Render/Resource/TextureLinker.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.TextureLinker", TextureLinker, Object)

TextureLinker::TextureLinker(TextureReader& textureReader)
:	m_textureReader(textureReader)
{
}

bool TextureLinker::link(const ShaderResource::Combination& shaderCombination, IProgram* program)
{
	const std::vector< Guid >& textures = shaderCombination.textures;
	for (uint32_t i = 0; i < textures.size(); ++i)
	{
		Ref< ITexture > texture = m_textureReader.read(textures[i]);
		if (!texture)
			return false;

		handle_t parameterHandle = getParameterHandleFromTextureReferenceIndex(i);
		program->setTextureParameter(parameterHandle, texture);
	}
	return true;
}

	}
}

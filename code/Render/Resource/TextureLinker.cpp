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
	for (std::vector< Guid >::const_iterator i = textures.begin(); i != textures.end(); ++i)
	{
		Ref< ITexture > texture = m_textureReader.read(*i);
		if (!texture)
			return false;

		handle_t parameterHandle = getParameterHandleFromGuid(*i);
		program->setTextureParameter(parameterHandle, texture);
	}
	return true;
}

	}
}

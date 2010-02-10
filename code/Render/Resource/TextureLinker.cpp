#include "Render/Shader.h"
#include "Render/Resource/ShaderResource.h"
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

bool TextureLinker::link(const ShaderResource* shaderResource, Shader* shader)
{
	const std::vector< Guid >& textures = shaderResource->getTextures();
	for (std::vector< Guid >::const_iterator i = textures.begin(); i != textures.end(); ++i)
	{
		resource::Proxy< ITexture > texture = m_textureReader.read(*i);
		if (texture)
		{
			std::wstring parameterName = getParameterNameFromGuid(*i);
			shader->setTextureParameter(parameterName, texture);
		}
	}
	return true;
}

	}
}

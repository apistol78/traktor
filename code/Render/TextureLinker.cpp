#include "Render/TextureLinker.h"
#include "Render/Shader.h"
#include "Render/ShaderResource.h"

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
	const std::vector< std::pair< std::wstring, Guid > >& textures = shaderResource->getTextures();
	for (std::vector< std::pair< std::wstring, Guid > >::const_iterator i = textures.begin(); i != textures.end(); ++i)
	{
		resource::Proxy< ITexture > texture = m_textureReader.read(i->second);
		if (texture)
			shader->setSamplerTexture(i->first, texture);
	}
	return true;
}

	}
}

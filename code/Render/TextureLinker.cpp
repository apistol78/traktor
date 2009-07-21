#include "Render/TextureLinker.h"
#include "Render/ProgramResource.h"
#include "Render/IProgram.h"
#include "Core/Heap/Ref.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.TextureLinker", TextureLinker, Object)

TextureLinker::TextureLinker(TextureReader& textureReader)
:	m_textureReader(textureReader)
{
}

bool TextureLinker::link(const ProgramResource* programResource, IProgram* program)
{
	const std::vector< std::pair< std::wstring, Guid > >& textures = programResource->getTextures();
	for (std::vector< std::pair< std::wstring, Guid > >::const_iterator i = textures.begin(); i != textures.end(); ++i)
	{
		Ref< ITexture > texture = m_textureReader.read(i->second);
		if (texture)
			program->setSamplerTexture(i->first, texture);
	}
	return true;
}

	}
}

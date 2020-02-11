#include "Core/Serialization/ISerializer.h"
#include "Render/ITexture.h"
#include "Render/Editor/Image2/ImgTexture.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const ImmutableNode::OutputPinDesc c_ImgTexture_o[] = { { L"Output" }, { 0 } };

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImgTexture", 0, ImgTexture, ImmutableNode)

ImgTexture::ImgTexture()
:	ImmutableNode(nullptr, c_ImgTexture_o)
{
}

const resource::Id< ITexture >& ImgTexture::getTexture() const
{
	return m_texture;
}

void ImgTexture::serialize(ISerializer& s)
{
	Node::serialize(s);
	s >> resource::Member< ITexture >(L"texture", m_texture);
}

	}
}
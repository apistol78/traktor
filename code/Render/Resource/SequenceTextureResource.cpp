#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStl.h"
#include "Render/ITexture.h"
#include "Render/Resource/SequenceTextureResource.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.SequenceTextureResource", 0, SequenceTextureResource, ISerializable)

SequenceTextureResource::SequenceTextureResource()
:	m_rate(0.0f)
{
}

void SequenceTextureResource::serialize(ISerializer& s)
{
	s >> Member< float >(L"rate", m_rate);
	s >> MemberStlList< resource::Id< ITexture >, resource::Member< ITexture > >(L"textures", m_textures);
}

	}
}

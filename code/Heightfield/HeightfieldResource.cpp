#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Heightfield/HeightfieldResource.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.HeightfieldResource", 0, HeightfieldResource, ISerializable)

HeightfieldResource::HeightfieldResource()
:	m_size(0)
,	m_worldExtent(0.0f, 0.0f, 0.0f, 0.0f)
{
}

bool HeightfieldResource::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"size", m_size);
	s >> Member< Vector4 >(L"worldExtent", m_worldExtent);
	return true;
}

	}
}

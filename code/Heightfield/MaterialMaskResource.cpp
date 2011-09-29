#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Heightfield/MaterialMaskResource.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.MaterialMaskResource", 0, MaterialMaskResource, ISerializable)

MaterialMaskResource::MaterialMaskResource(uint32_t size)
:	m_size(size)
{
}

uint32_t MaterialMaskResource::getSize() const
{
	return m_size;
}

bool MaterialMaskResource::serialize(ISerializer& s)
{
	return s >> Member< uint32_t >(L"size", m_size);
}

	}
}

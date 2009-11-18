#include "Terrain/MaterialMaskResource.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.MaterialMaskResource", MaterialMaskResource, ISerializable)

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

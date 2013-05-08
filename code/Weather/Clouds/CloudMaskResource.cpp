#include "Weather/Clouds/CloudMaskResource.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace weather
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.CloudMaskResource", 0, CloudMaskResource, ISerializable)

CloudMaskResource::CloudMaskResource(int32_t size)
:	m_size(size)
{
}

int32_t CloudMaskResource::getSize() const
{
	return m_size;
}

void CloudMaskResource::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"size", m_size);
}

	}
}

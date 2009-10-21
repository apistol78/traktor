#include "Weather/Clouds/CloudMaskResource.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace weather
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.terrain.CloudMaskResource", CloudMaskResource, Serializable)

CloudMaskResource::CloudMaskResource(int32_t size)
:	m_size(size)
{
}

int32_t CloudMaskResource::getSize() const
{
	return m_size;
}

bool CloudMaskResource::serialize(Serializer& s)
{
	return s >> Member< int32_t >(L"size", m_size);
}

	}
}

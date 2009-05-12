#include "Physics/HeightfieldResource.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.physics.HeightfieldResource", HeightfieldResource, Serializable)

HeightfieldResource::HeightfieldResource(uint32_t size, const Vector4& worldExtent)
:	m_size(size)
,	m_worldExtent(worldExtent)
{
}

uint32_t HeightfieldResource::getSize() const
{
	return m_size;
}

const Vector4& HeightfieldResource::getWorldExtent() const
{
	return m_worldExtent;
}

bool HeightfieldResource::serialize(Serializer& s)
{
	s >> Member< uint32_t >(L"size", m_size);
	s >> Member< Vector4 >(L"worldExtent", m_worldExtent);
	return true;
}

	}
}

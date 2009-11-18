#include "Physics/HeightfieldResource.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.HeightfieldResource", HeightfieldResource, ISerializable)

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

bool HeightfieldResource::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"size", m_size);
	s >> Member< Vector4 >(L"worldExtent", m_worldExtent);
	return true;
}

	}
}

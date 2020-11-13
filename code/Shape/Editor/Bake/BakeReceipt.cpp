#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Shape/Editor/Bake/BakeReceipt.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.BakeReceipt", 0, BakeReceipt, ISerializable)

void BakeReceipt::setTransform(const Guid& entityId, const Transform& transform)
{
	m_lastKnownTransforms[entityId] = transform;
}

bool BakeReceipt::getLastKnownTransform(const Guid& entityId, Transform& outLastTransform) const
{
	auto it = m_lastKnownTransforms.find(entityId);
	if (it != m_lastKnownTransforms.end())
	{
		outLastTransform = it->second;
		return true;
	}
	else
		return false;
}

void BakeReceipt::serialize(ISerializer& s)
{
	s >> MemberSmallMap< Guid, Transform, Member< Guid >, MemberComposite< Transform > >(L"lastKnownTransforms", m_lastKnownTransforms);
}

	}
}

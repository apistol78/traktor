#pragma once

#include "Core/Guid.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Math/Transform.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace shape
	{

class BakeReceipt : public ISerializable
{
	T_RTTI_CLASS;

public:
	void setTransform(const Guid& entityId, const Transform& transform);

	bool getLastKnownTransform(const Guid& entityId, Transform& outLastTransform) const;	

	virtual void serialize(ISerializer& s) override final;

private:
	SmallMap< Guid, Transform > m_lastKnownTransforms;
};

	}
}

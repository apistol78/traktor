#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Transform.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MODEL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace model
	{

/*! \brief Pose
 * \ingroup Model
 */
class T_DLLCLASS Pose : public ISerializable
{
	T_RTTI_CLASS;

public:
	void setJointTransform(uint32_t jointId, const Transform& jointTransform);

	const Transform& getJointTransform(uint32_t jointId) const;

	virtual void serialize(ISerializer& s) override final;

private:
	AlignedVector< Transform > m_jointTransforms;
};

	}
}

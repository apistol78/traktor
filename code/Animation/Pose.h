#ifndef traktor_animation_Pose_H
#define traktor_animation_Pose_H

#include "Animation/BitSet.h"
#include "Animation/Rotator.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

/*! \brief Skeleton pose.
 * \ingroup Animation
 */
class T_DLLCLASS Pose : public ISerializable
{
	T_RTTI_CLASS;

public:
	void setJointOffset(uint32_t jointIndex, const Vector4& jointOffset);

	Vector4 getJointOffset(uint32_t jointIndex) const;

	void setJointOrientation(uint32_t jointIndex, const Rotator& jointOrientation);

	Rotator getJointOrientation(uint32_t jointIndex) const;

	uint32_t getMaxIndex() const;

	void getIndexMask(BitSet& outIndices) const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	struct Joint
	{
		uint32_t index;
		Vector4 offset;
		Rotator orientation;

		Joint(uint32_t index_ = 0)
		:	index(index_)
		,	offset(0.0f, 0.0f, 0.0f, 0.0f)
		{
		}

		void serialize(ISerializer& s);
	};

	AlignedVector< Joint > m_joints;

	const Joint* getJoint(uint32_t jointIndex) const;

	Joint& getEditJoint(uint32_t jointIndex);
};

	}
}

#endif	// traktor_animation_Pose_H

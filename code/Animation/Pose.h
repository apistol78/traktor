#ifndef traktor_animation_Pose_H
#define traktor_animation_Pose_H

#include "Animation/BitSet.h"
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
	void setBoneOffset(uint32_t boneIndex, const Vector4& boneOffset);

	Vector4 getBoneOffset(uint32_t boneIndex) const;

	void setBoneOrientation(uint32_t boneIndex, const Vector4& boneOrientation);

	Vector4 getBoneOrientation(uint32_t boneIndex) const;

	void getIndexMask(BitSet& outIndices) const;

	virtual bool serialize(ISerializer& s);

private:
	struct Bone
	{
		uint32_t index;
		Vector4 offset;
		Vector4 orientation;

		Bone(uint32_t index_ = 0)
		:	index(index_)
		,	offset(0.0f, 0.0f, 0.0f, 0.0f)
		,	orientation(0.0f, 0.0f, 0.0f, 0.0f)
		{
		}

		bool serialize(ISerializer& s);
	};

	AlignedVector< Bone > m_bones;

	const Bone* getBone(uint32_t boneIndex) const;

	Bone& getEditBone(uint32_t boneIndex);
};

	}
}

#endif	// traktor_animation_Pose_H

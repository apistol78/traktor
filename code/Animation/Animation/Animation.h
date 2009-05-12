#ifndef traktor_animation_Animation_H
#define traktor_animation_Animation_H

#include "Core/Serialization/Serializable.h"
#include "Core/Containers/AlignedVector.h"
#include "Animation/Pose.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

/*! \brief Key framed animation poses.
 * \ingroup Animation
 */
class T_DLLCLASS Animation : public Serializable
{
	T_RTTI_CLASS(Animation)

public:
	struct T_DLLCLASS KeyPose
	{
		float at;
		Pose pose;

		bool serialize(Serializer& s);
	};

	uint32_t addKeyPose(const KeyPose& pose);

	void removeKeyPose(uint32_t poseIndex);

	bool empty() const;

	uint32_t getKeyPoseCount() const;

	KeyPose& getKeyPose(uint32_t poseIndex);

	const KeyPose& getKeyPose(uint32_t poseIndex) const;

	const KeyPose& getLastKeyPose() const;

	bool getPose(float at, Pose& outPose) const;

	virtual bool serialize(Serializer& s);

private:
	AlignedVector< KeyPose > m_poses;
};

	}
}

#endif	// traktor_animation_Animation_H

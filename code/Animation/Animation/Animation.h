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

	/*! \brief Add key pose to animation.
	 *
	 * \param pose Key pose.
	 * \return Index of added key pose.
	 */
	uint32_t addKeyPose(const KeyPose& pose);

	/*! \brief Remove key pose from animation.
	 *
	 * \param poseIndex Index of key pose.
	 */
	void removeKeyPose(uint32_t poseIndex);

	/*! \brief Return true if animation doesn't contain any poses.
	 *
	 * \return True if animation is empty.
	 */
	bool empty() const;

	/*! \brief Get number of key poses in animation.
	 *
	 * \return Number of key poses.
	 */
	uint32_t getKeyPoseCount() const;

	/*! \brief Get reference to key pose.
	 *
	 * \param poseIndex Index of key pose.
	 * \return Key pose.
	 */
	KeyPose& getKeyPose(uint32_t poseIndex);

	/*! \brief Get reference to key pose.
	 *
	 * \param poseIndex Index of key pose.
	 * \return Key pose.
	 */
	const KeyPose& getKeyPose(uint32_t poseIndex) const;

	/*! \brief Get last key pose.
	 *
	 * \return Last key pose.
	 */
	const KeyPose& getLastKeyPose() const;

	/*! \brief Get key pose from time.
	 *
	 * \param at Time
	 * \param outPose Output key pose.
	 * \return True if found key pose.
	 */
	bool getPose(float at, Pose& outPose) const;

	virtual bool serialize(Serializer& s);

private:
	AlignedVector< KeyPose > m_poses;
};

	}
}

#endif	// traktor_animation_Animation_H

#ifndef traktor_animation_Skeleton_H
#define traktor_animation_Skeleton_H

#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Serialization/ISerializable.h"
#include "Render/Types.h"

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

class Joint;

/*! \brief Animation skeleton.
 * \ingroup Animation
 */
class T_DLLCLASS Skeleton : public ISerializable
{
	T_RTTI_CLASS;

public:
	int32_t addJoint(Joint* joint);

	void removeJoint(Joint* joint);

	bool findJoint(render::handle_t name, uint32_t& outIndex) const;

	void findChildren(uint32_t index, std::vector< uint32_t >& outChildren) const;

	virtual void serialize(ISerializer& s);

	uint32_t getJointCount() const { return uint32_t(m_joints.size()); }

	Joint* getJoint(uint32_t index) const { return m_joints[index]; }

private:
	RefArray< Joint > m_joints;
	mutable SmallMap< render::handle_t, uint32_t > m_jointMap;
};

	}
}

#endif	// traktor_animation_Skeleton_H

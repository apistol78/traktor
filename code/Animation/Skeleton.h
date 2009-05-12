#ifndef traktor_animation_Skeleton_H
#define traktor_animation_Skeleton_H

#include "Core/Heap/Ref.h"
#include "Core/Serialization/Serializable.h"

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

class Bone;

/*! \brief Animation skeleton.
 * \ingroup Animation
 */
class T_DLLCLASS Skeleton : public Serializable
{
	T_RTTI_CLASS(Skeleton)

public:
	uint32_t addBone(Bone* bone);

	void removeBone(Bone* bone);

	bool findBone(const std::wstring& name, uint32_t& outIndex) const;

	virtual bool serialize(Serializer& s);

	inline uint32_t getBoneCount() const { return uint32_t(m_bones.size()); }

	inline Bone* getBone(uint32_t index) const { return m_bones[index]; }

private:
	RefArray< Bone > m_bones;
};

	}
}

#endif	// traktor_animation_Skeleton_H

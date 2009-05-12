#ifndef traktor_animation_Path_H
#define traktor_animation_Path_H

#include "Core/Serialization/Serializable.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Quaternion.h"

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

/*! \brief Movement path.
 * \ingroup Animation
 */
class T_DLLCLASS Path : public Serializable
{
	T_RTTI_CLASS(Path)

public:
	struct Frame
	{
		Vector4 position;
		Quaternion orientation;

		Frame()
		:	position(Vector4::origo())
		,	orientation(Quaternion::identity())
		{
		}

		bool serialize(Serializer& s);
	};

	struct Key
	{
		float T;
		Frame value;

		Key()
		:	T(0.0f)
		{
		}

		bool serialize(Serializer& s);
	};

	void insert(float at, const Frame& frame);

	Frame evaluate(float at) const;

	Frame* getClosestKeyFrame(float at);

	virtual bool serialize(Serializer& s);

	inline float getEndTime() const { return !m_keys.empty() ? m_keys.back().T : 0.0f; }

	inline const AlignedVector< Key >& getKeys() const { return m_keys; }

private:
	AlignedVector< Key > m_keys;
};

	}
}

#endif	// traktor_animation_Path_H

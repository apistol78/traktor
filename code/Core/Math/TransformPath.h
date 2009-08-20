#ifndef traktor_TransformPath_H
#define traktor_TransformPath_H

#include "Core/Serialization/Serializable.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Quaternion.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Transformation path.
 * \ingroup Core
 */
class T_DLLCLASS TransformPath : public Serializable
{
	T_RTTI_CLASS(TransformPath)

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

	Key* getClosestKey(float at);

	Key* getClosestPreviousKey(float at);

	Key* getClosestNextKey(float at);

	Frame* getClosestKeyFrame(float at);

	virtual bool serialize(Serializer& s);

	inline float getStartTime() const { return !m_keys.empty() ? m_keys.front().T : 0.0f; }

	inline float getEndTime() const { return !m_keys.empty() ? m_keys.back().T : 0.0f; }

	inline const AlignedVector< Key >& getKeys() const { return m_keys; }

private:
	AlignedVector< Key > m_keys;
};

}

#endif	// traktor_TransformPath_H

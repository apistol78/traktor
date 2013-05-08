#ifndef traktor_TransformPath_H
#define traktor_TransformPath_H

#include <vector>
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/ISpline.h"
#include "Core/Math/Transform.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Transformation path.
 * \ingroup Core
 */
class T_DLLCLASS TransformPath : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct T_DLLCLASS Frame
	{
		Vector4 position;
		Vector4 orientation;

		Frame()
		:	position(Vector4::origo())
		,	orientation(Vector4::zero())
		{
		}
		
		virtual ~Frame() {}

		Transform transform() const;

		void serialize(ISerializer& s);
	};

	struct Key
	{
		float T;
		Vector4 tcb;
		Frame value;

		Key()
		:	T(0.0f)
		,	tcb(0.0f, 0.0f, 0.0f, 0.0f)
		{
		}
		
		virtual ~Key() {}

		void serialize(ISerializer& s);
	};

	TransformPath();

	TransformPath(const TransformPath& path);

	void insert(float at, const Frame& frame);

	Frame evaluate(float at) const;

	Frame evaluate(float at, float end) const;

	Frame evaluate(float at, float end, float loop) const;

	Key* getClosestKey(float at);

	Key* getClosestPreviousKey(float at);

	Key* getClosestNextKey(float at);

	Frame* getClosestKeyFrame(float at);

	virtual void serialize(ISerializer& s);

	float getStartTime() const { return !m_keys.empty() ? m_keys.front().T : 0.0f; }

	float getEndTime() const { return !m_keys.empty() ? m_keys.back().T : 0.0f; }

	const AlignedVector< Key >& getKeys() const { return m_keys; }

	AlignedVector< Key >& getKeys() { return m_keys; }

private:
	AlignedVector< Key > m_keys;
	mutable AutoPtr< ISpline< Frame > > m_spline;
};

}

#endif	// traktor_TransformPath_H

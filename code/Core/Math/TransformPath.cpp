#include <limits>
#include "Core/Math/Hermite.h"
#include "Core/Math/Float.h"
#include "Core/Math/TcbSpline.h"
#include "Core/Math/TransformPath.h"
#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/AttributePoint.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace
	{

/*! \brief Open uniform TCB spline accessor. */
class T_MATH_ALIGN16 OpenUniformAccessor
{
public:
	OpenUniformAccessor(const AlignedVector< TransformPath::Key >& keys)
	:	m_keys(keys)
	{
	}

	int32_t index(float T, float Tend) const
	{
		int32_t i = m_keys.size() - 1;
		for (; i > 0; --i)
		{
			if (T >= m_keys[i].T)
				break;
		}
		return i;
	}

	float time(int32_t index) const
	{
		return Scalar(key(index).T);
	}

	float tension(int32_t index) const
	{
		return key(index).tcb.x();
	}

	float continuity(int32_t index) const
	{
		return key(index).tcb.y();
	}

	float bias(int32_t index) const
	{
		return key(index).tcb.z();
	}

	const TransformPath::Frame& value(int32_t index) const
	{
		return key(index).value;
	}

	TransformPath::Frame combine(
		const TransformPath::Frame& v0, float w0,
		const TransformPath::Frame& v1, float w1,
		const TransformPath::Frame& v2, float w2,
		const TransformPath::Frame& v3, float w3
	) const
	{
		TransformPath::Frame f;
		f.position = v0.position * Scalar(w0) + v1.position * Scalar(w1) + v2.position * Scalar(w2) + v3.position * Scalar(w3);
		f.orientation = v0.orientation * Scalar(w0) + v1.orientation * Scalar(w1) + v2.orientation * Scalar(w2) + v3.orientation * Scalar(w3);
		return f;
	}

private:
	const AlignedVector< TransformPath::Key >& m_keys;

	const TransformPath::Key& key(int32_t index) const
	{
		int32_t nkeys = int32_t(m_keys.size());
		if (index < 0)
			index = 0;
		if (index >= nkeys)
			index = nkeys - 1;
		return m_keys[index];
	}
};

/*! \brief Closed uniform TCB spline accessor. */
class T_MATH_ALIGN16 ClosedUniformAccessor
{
public:
	ClosedUniformAccessor(const AlignedVector< TransformPath::Key >& keys)
	:	m_keys(keys)
	{
	}

	int32_t index(float T, float Tend) const
	{
		int32_t nkeys = int32_t(m_keys.size());

		float Tn = T;
		while (Tn < 0.0f)
			Tn += Tend;
		while (Tn > Tend)
			Tn -= Tend;

		int32_t i = nkeys - 1;
		for (; i >= 0; --i)
		{
			if (Tn >= m_keys[i].T)
				return i;
		}

		return nkeys - 1;
	}

	float time(int32_t index) const
	{
		return Scalar(key(index).T);
	}

	float tension(int32_t index) const
	{
		return key(index).tcb.x();
	}

	float continuity(int32_t index) const
	{
		return key(index).tcb.y();
	}

	float bias(int32_t index) const
	{
		return key(index).tcb.z();
	}

	const TransformPath::Frame& value(int32_t index) const
	{
		return key(index).value;
	}

	TransformPath::Frame combine(
		const TransformPath::Frame& v0, float w0,
		const TransformPath::Frame& v1, float w1,
		const TransformPath::Frame& v2, float w2,
		const TransformPath::Frame& v3, float w3
	) const
	{
		TransformPath::Frame f;
		f.position = v0.position * Scalar(w0) + v1.position * Scalar(w1) + v2.position * Scalar(w2) + v3.position * Scalar(w3);
		f.orientation = v0.orientation * Scalar(w0) + v1.orientation * Scalar(w1) + v2.orientation * Scalar(w2) + v3.orientation * Scalar(w3);
		return f;
	}

private:
	const AlignedVector< TransformPath::Key >& m_keys;

	const TransformPath::Key& key(int32_t index) const
	{
		int32_t nkeys = int32_t(m_keys.size());
		while (index < 0)
			index += nkeys;
		index %= nkeys;
		return m_keys[index];
	}
};

struct PairAccessor
{
	static inline float time(const std::pair< float, float >& key) { return key.first; }
	
	static inline float value(const std::pair< float, float >& key) { return key.second; }

	static inline float combine(
		float v0, float w0,
		float v1, float w1,
		float v2, float w2,
		float v3, float w3
	)
	{
		return float(v0 * w0 + v1 * w1 + v2 * w2 + v3 * w3);
	}
};

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.TransformPath", 0, TransformPath, ISerializable)

TransformPath::TransformPath()
:	m_loop(false)
{
}

TransformPath::TransformPath(const TransformPath& path)
:	m_keys(path.m_keys)
,	m_loop(false)
{
}

void TransformPath::insert(float at, const Frame& frame)
{
	Key key;
	key.T = at;
	key.value = frame;

	size_t keys = m_keys.size();
	if (keys >= 1)
	{
		if (at <= m_keys.front().T)
			m_keys.insert(m_keys.begin(), key);
		else if (at >= m_keys.back().T)
			m_keys.insert(m_keys.end(), key);
		else
		{
			for (size_t i = 0; i < keys - 1; ++i)
			{
				const Key& k1 = m_keys[i];
				const Key& k2 = m_keys[i + 1];
				if (at > k1.T && at < k2.T)
				{
					AlignedVector< Key >::iterator iter = m_keys.begin(); std::advance(iter, i + 1);
					m_keys.insert(iter, key);
					break;
				}
			}
		}
	}
	else
	{
		m_keys.push_back(key);
	}

	m_spline.release();
}

TransformPath::Frame TransformPath::evaluate(float at, bool loop) const
{
	float Tend = getEndTime() + getStartTime();
	return evaluate(at, Tend, loop);
}

TransformPath::Frame TransformPath::evaluate(float at, float end, bool loop) const
{
	size_t nkeys = m_keys.size();
	if (nkeys == 0)
		return Frame();
	else if (nkeys == 1)
		return m_keys[0].value;
	else
	{
		if (!m_spline.ptr() || m_loop != loop)
		{
			if (loop)
			{
				m_spline.reset(new TcbSpline< Key, Frame, ClosedUniformAccessor >(
					ClosedUniformAccessor(m_keys),
					end
				));
			}
			else
			{
				m_spline.reset(new TcbSpline< Key, Frame, OpenUniformAccessor >(
					OpenUniformAccessor(m_keys),
					end
				));
			}

			m_loop = loop;
		}

		if (m_loop)
		{
			while (at > end)
				at -= end;
		}
		else
		{
			float begin = m_keys.front().T;
			if (at < begin)
				at = begin;
			if (at > end)
				at = end;
		}

		Frame frame = m_spline->evaluate(at);
		frame.position = frame.position.xyz1();
		frame.orientation = frame.orientation.xyz0();

		return frame;
	}
}

TransformPath::Key* TransformPath::getClosestKey(float at)
{
	if (m_keys.empty())
		return 0;

	float minT = std::numeric_limits< float >::max();
	uint32_t minI = 0;

	for (uint32_t i = 0; i < uint32_t(m_keys.size()); ++i)
	{
		float dT = abs(at - m_keys[i].T);
		if (dT < minT)
		{
			minT = dT;
			minI = i;
		}
	}

	return &m_keys[minI];
}

TransformPath::Key* TransformPath::getClosestPreviousKey(float at)
{
	if (m_keys.empty())
		return 0;

	float minT = std::numeric_limits< float >::max();
	Key* minK = 0;

	for (uint32_t i = 0; i < uint32_t(m_keys.size()); ++i)
	{
		if (at <= m_keys[i].T + FUZZY_EPSILON)
			continue;

		float dT = at - m_keys[i].T;
		if (dT < minT)
		{
			minT = dT;
			minK = &m_keys[i];
		}
	}

	return minK;
}

TransformPath::Key* TransformPath::getClosestNextKey(float at)
{
	if (m_keys.empty())
		return 0;

	float minT = std::numeric_limits< float >::max();
	Key* minK = 0;

	for (uint32_t i = 0; i < uint32_t(m_keys.size()); ++i)
	{
		if (at >= m_keys[i].T - FUZZY_EPSILON)
			continue;

		float dT = m_keys[i].T - at;
		if (dT < minT)
		{
			minT = dT;
			minK = &m_keys[i];
		}
	}

	return minK;
}

TransformPath::Frame* TransformPath::getClosestKeyFrame(float at)
{
	TransformPath::Key* closestKey = getClosestKey(at);
	return closestKey ? &closestKey->value : 0;
}

bool TransformPath::serialize(ISerializer& s)
{
	return s >> MemberAlignedVector< Key, MemberComposite< Key > >(L"keys", m_keys);
}

bool TransformPath::Key::serialize(ISerializer& s)
{
	s >> Member< float >(L"T", T, AttributeRange(0.0f));
	s >> Member< Vector4 >(L"tcb", tcb);
	s >> MemberComposite< Frame >(L"value", value);
	return true;
}

Transform TransformPath::Frame::transform() const
{
	return Transform(
		position.xyz0(),
		Quaternion::fromEulerAngles(
			orientation.x(),
			orientation.y(),
			orientation.z()
		)
	);
}

bool TransformPath::Frame::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"position", position, AttributePoint());
	s >> Member< Vector4 >(L"orientation", orientation, AttributeDirection());
	return true;
}

}

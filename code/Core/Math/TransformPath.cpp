#include <limits>
#include "Core/Math/TcbSpline.h"
#include "Core/Math/TransformPath.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace
	{

Quaternion orientationAsQuaternion(const Vector4& orientation)
{
	return Quaternion(orientation.x(), orientation.y(), orientation.z());
}

/*! \brief Closed uniform TCB spline accessor.
 *
 * Open : 0->1->2->...->N
 * Closed : 0->1->2->...->N->0
 *
 */
class ClosedUniformAccessor
{
public:
	ClosedUniformAccessor(const AlignedVector< TransformPath::Key >& keys)
	:	m_keys(keys)
	{
	}

	int32_t index(const Scalar& T) const
	{
		int32_t nkeys = int32_t(m_keys.size());
		Scalar Tn = T;
		while (Tn < 0.0f)
			Tn += Scalar(1.0f);
		while (Tn > 1.0f)
			Tn -= Scalar(1.0f);
		return int32_t(Tn * nkeys);
	}

	int32_t index(int32_t i) const
	{
		int32_t nkeys = int32_t(m_keys.size());
		while (i < 0)
			i += nkeys;
		while (i > nkeys)
			i -= nkeys;
		return i;
	}

	Scalar time(int32_t index) const
	{
		int32_t nkeys = int32_t(m_keys.size());
		return Scalar(float(index) / nkeys);
	}

	Scalar tension(int32_t index) const
	{
		int32_t nkeys = int32_t(m_keys.size());
		return m_keys[index % nkeys].tcb.x();
	}

	Scalar continuity(int32_t index) const
	{
		int32_t nkeys = int32_t(m_keys.size());
		return m_keys[index % nkeys].tcb.y();
	}

	Scalar bias(int32_t index) const
	{
		int32_t nkeys = int32_t(m_keys.size());
		return m_keys[index % nkeys].tcb.z();
	}

	const TransformPath::Frame& value(int32_t index) const
	{
		int32_t nkeys = int32_t(m_keys.size());
		return m_keys[index % nkeys].value;
	}

	TransformPath::Frame combine(
		const TransformPath::Frame& v0, const Scalar& w0,
		const TransformPath::Frame& v1, const Scalar& w1,
		const TransformPath::Frame& v2, const Scalar& w2,
		const TransformPath::Frame& v3, const Scalar& w3
	) const
	{
		TransformPath::Frame f;
		f.position = v0.position * w0 + v1.position * w1 + v2.position * w2 + v3.position * w3;
		f.orientation = v0.orientation * w0 + v1.orientation * w1 + v2.orientation * w2 + v3.orientation * w3;
		return f;
	}

private:
	const AlignedVector< TransformPath::Key >& m_keys;
};

/*! \brief Closed timed TCB spline accessor.
 *
 * Open : 0->1->2->...->N
 * Closed : 0->1->2->...->N->0
 *
 */
class ClosedTimedAccessor
{
public:
	ClosedTimedAccessor(const AlignedVector< TransformPath::Key >& keys, const Scalar& Tend)
	:	m_keys(keys)
	,	m_Tend(Tend)
	{
	}

	int32_t index(const Scalar& T) const
	{
		int32_t nkeys = int32_t(m_keys.size());

		Scalar Tfront = Scalar(m_keys.front().T);
		Scalar Tduration = m_Tend - Tfront;

		Scalar Tn = T - Tduration;
		while (Tn > m_Tend - Tduration)
			Tn -= m_Tend - Tduration;

		int32_t i = 0;
		for (; i < nkeys; ++i)
		{
			if (Tn < m_keys[i].T - Tfront)
				break;
		}

		return i;
	}

	int32_t index(int32_t i) const
	{
		int32_t nkeys = int32_t(m_keys.size());
		while (i < 0)
			i += nkeys;
		while (i > nkeys)
			i -= nkeys;
		return i;
	}

	Scalar time(int32_t index) const
	{
		int32_t nkeys = int32_t(m_keys.size());
		return Scalar(float(index) / nkeys);
	}

	Scalar tension(int32_t index) const
	{
		int32_t nkeys = int32_t(m_keys.size());
		return m_keys[index % nkeys].tcb.x();
	}

	Scalar continuity(int32_t index) const
	{
		int32_t nkeys = int32_t(m_keys.size());
		return m_keys[index % nkeys].tcb.y();
	}

	Scalar bias(int32_t index) const
	{
		int32_t nkeys = int32_t(m_keys.size());
		return m_keys[index % nkeys].tcb.z();
	}

	const TransformPath::Frame& value(int32_t index) const
	{
		int32_t nkeys = int32_t(m_keys.size());
		return m_keys[index % nkeys].value;
	}

	TransformPath::Frame combine(
		const TransformPath::Frame& v0, const Scalar& w0,
		const TransformPath::Frame& v1, const Scalar& w1,
		const TransformPath::Frame& v2, const Scalar& w2,
		const TransformPath::Frame& v3, const Scalar& w3
	) const
	{
		TransformPath::Frame f;
		f.position = v0.position * w0 + v1.position * w1 + v2.position * w2 + v3.position * w3;
		f.orientation = v0.orientation * w0 + v1.orientation * w1 + v2.orientation * w2 + v3.orientation * w3;
		return f;
	}

private:
	const AlignedVector< TransformPath::Key >& m_keys;
	Scalar m_Tend;
};

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.TransformPath", 0, TransformPath, ISerializable)

TransformPath::TransformPath()
{
}

TransformPath::TransformPath(const TransformPath& path)
:	m_keys(path.m_keys)
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
		// Create spline evaluator and arclength table.
		if (!m_spline.ptr())
		{
			// Open : 0->1->2->...->N
			// Closed : 0->1->2->...->N->0
			m_spline.reset(new TcbSpline< Key, Scalar, Scalar, Frame, ClosedUniformAccessor >(
				ClosedUniformAccessor(m_keys)
			));

			int32_t narc = int32_t(m_keys.size() * 10);
			m_arcLengths.resize(narc);

			Vector4 last = m_spline->evaluate(Scalar(0.0f), Scalar(1.0f)).position;
			float totalLength = 0.0f;

			for (int32_t i = 0; i < narc; ++i)
			{
				float T = float(i + 1) / narc;
				Vector4 current = m_spline->evaluate(Scalar(T), Scalar(1.0f)).position;
				float length = (current - last).length();
				m_arcLengths[i] = totalLength;
				totalLength += length;
				last = current;
			}

			m_spline.reset(new TcbSpline< Key, Scalar, Scalar, Frame, ClosedTimedAccessor >(
				ClosedTimedAccessor(m_keys, Scalar(end))
			));
		}

		float tf = m_keys[0].T;
		float tb = end; //m_keys.back().T;

		// Estimate target arc length.
		float at0 = std::fmod((at - tf) / (tb - tf), 1.0f);
		float targetArcLength = at0 * m_arcLengths.back();

		// Find index into arc length vector.
		int32_t index = 0;
		for (int32_t i = 1; i < int32_t(m_arcLengths.size()); ++i)
		{
			if (m_arcLengths[i] > targetArcLength)
			{
				index = i - 1;
				break;
			}
		}

		// Calculate parameterized t.
		float ln0 = m_arcLengths[index];
		float ln1 = m_arcLengths[index + 1];
		float f = (targetArcLength - ln0) / (ln1 - ln0);
		float t = tf + (end - tf) * (index + f) / float(m_arcLengths.size() - 1);

		Frame frame = m_spline->evaluate(Scalar(t), Scalar(end));
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
		Quaternion(
			orientation.x(),
			orientation.y(),
			orientation.z()
		)
	);
}

bool TransformPath::Frame::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"position", position);
	s >> Member< Vector4 >(L"orientation", orientation);
	return true;
}

}

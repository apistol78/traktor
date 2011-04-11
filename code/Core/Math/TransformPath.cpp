#include <limits>
#include "Core/Math/Hermite.h"
#include "Core/Math/Float.h"
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

/*! \brief Open uniform TCB spline accessor. */
class T_MATH_ALIGN16 OpenUniformAccessor
{
public:
	OpenUniformAccessor(const AlignedVector< TransformPath::Key >& keys)
	:	m_keys(keys)
	{
	}

	int32_t index(const Scalar& T) const
	{
		int32_t nkeys = int32_t(m_keys.size());
		return clamp(int32_t(T * nkeys), 0, nkeys - 1);
	}

	Scalar time(int32_t index) const
	{
		int32_t nkeys = int32_t(m_keys.size());
		return Scalar(float(index) / nkeys);
	}

	Scalar tension(int32_t index) const
	{
		return key(index).tcb.x();
	}

	Scalar continuity(int32_t index) const
	{
		return key(index).tcb.y();
	}

	Scalar bias(int32_t index) const
	{
		return key(index).tcb.z();
	}

	const TransformPath::Frame& value(int32_t index) const
	{
		return key(index).value;
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

	Scalar time(int32_t index) const
	{
		int32_t nkeys = int32_t(m_keys.size());
		return Scalar(float(index) / nkeys);
	}

	Scalar tension(int32_t index) const
	{
		return key(index).tcb.x();
	}

	Scalar continuity(int32_t index) const
	{
		return key(index).tcb.y();
	}

	Scalar bias(int32_t index) const
	{
		return key(index).tcb.z();
	}

	const TransformPath::Frame& value(int32_t index) const
	{
		return key(index).value;
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
		const float& v0, const float& w0,
		const float& v1, const float& w1,
		const float& v2, const float& w2,
		const float& v3, const float& w3
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
		const int32_t c_arcLengthSteps = 100;

		if (loop)
		{
			while (at >= end)
				at -= end;
		}
		else
		{
			if (at > m_keys.back().T)
				at = m_keys.back().T;
		}

		float Tfirst = m_keys[0].T;
		float Tlast = m_keys.back().T - Tfirst;
		float Tat = at - Tfirst;
		float Tend = end - Tfirst;

		if (m_loop)
		{
			while (Tat < 0.0f)
				Tat += Tend;
		}
		else
		{
			if (Tat < 0.0f)
				Tat = 0.0f;
		}

		int32_t nkeys = int32_t(m_keys.size());

		// Create spline evaluator and arclength table.
		if (!m_spline.ptr() || m_loop != loop)
		{
			if (loop)
			{
				m_spline.reset(new TcbSpline< Key, Scalar, Scalar, Frame, ClosedUniformAccessor >(
					ClosedUniformAccessor(m_keys)
				));
			}
			else
			{
				m_spline.reset(new TcbSpline< Key, Scalar, Scalar, Frame, OpenUniformAccessor >(
					OpenUniformAccessor(m_keys)
				));
			}

			m_loop = loop;

			// Calculate arc-length table.
			int32_t narc = nkeys * c_arcLengthSteps;

			Frame last = m_spline->evaluate(Scalar(0.0f), Scalar(1.0f));
			float totalLength = 0.0f;

			m_arcLengths.resize(narc);
			for (int32_t i = 0; i < narc; ++i)
			{
				float T = float(i + 1) / narc;
				Frame current = m_spline->evaluate(Scalar(T), Scalar(1.0f));

				Frame delta;
				delta.position = current.position - last.position;
				delta.orientation = current.orientation - last.orientation;

				float linearDistance = delta.position.length();
				float orientationDistance = delta.orientation.length();

				m_arcLengths[i] = totalLength;
				totalLength += linearDistance;
				last = current;
			}
			m_arcLengths.push_back(m_arcLengths.back() + m_arcLengths[1]);

			m_velocities.resize(nkeys);
			for (int32_t i = 0; i < nkeys; ++i)
			{
				float aln0 = m_arcLengths[i * c_arcLengthSteps];
				float aln1 = m_arcLengths[i * c_arcLengthSteps + c_arcLengthSteps];

				float tc = m_keys[i].T;
				float tn = i < nkeys - 1 ? m_keys[i + 1].T : Tend;

				float dT = tn - tc;
				T_ASSERT (dT > 0.0f);

				float v = (aln1 - aln0) / dT;
				m_velocities[i] = v;
			}
		}

		// Calculate T on spline from given time "at".
		float Tln = 0.0f;
		for (int32_t i = nkeys - 1; i >= 0; --i)
		{
			float Tcurr = m_keys[i].T - Tfirst;
			float Tnext = (i < nkeys - 1) ? (m_keys[i + 1].T - Tfirst): Tend;

			if (Tat >= Tcurr)
			{
				int32_t iln0 = i * c_arcLengthSteps;
				int32_t iln1 = iln0 + c_arcLengthSteps;

				float ln0 = m_arcLengths[iln0];
				float ln1 = m_arcLengths[iln1];

				float f = (Tat - Tcurr) / (Tnext - Tcurr);

				// Estimate velocity at key points.
				float v0 = m_velocities[i];
				float v1 = (i < nkeys - 1) ? m_velocities[i + 1] : v0;

				float A = v0 + v1 + 2.0f * (ln0 - ln1);
				float B = 3.0f * (ln1 - ln0) - v1 - 2.0f * v0;
				float C = v0;
				float D = ln0;

				float fln2 = A * (f * f * f) + B * (f * f) + C * f + D;

				//float a0 = 0.0f;
				//float a1 = 0.0f;

				//float A = -332.0f * ln1 / 78.0f - 21.0f * a0 / 78.0f + 86.0f * v0 / 78.0f + 46.0f * v1 / 78.0f + 132.0f * ln0 / 78 - a1 / 78.0f;
				//float B = 430.0f * ln1 / 78.0f - 3.0f * a0 / 78.0f - 16.0f * v0 / 78.0f - 14.0f * v1 / 78.0f - 30.0f * ln0 / 78.0f + 2.0f * a1 / 78.0f;
				//float C = -20.0f * ln1 / 78.0f - 57.0f * a0 / 78.0f - 148.0f * v0 / 78.0f - 180.0f * ln0 / 78.0f - 32.0f * v1 / 78.0f - a1 / 78.0f;
				//float D = a0 / 2.0f;
				//float E = v0;
				//float F = ln0;

				//float f2 = f * f;
				//float f3 = f2 * f;
				//float f4 = f3 * f;
				//float f5 = f4 * f;
				//float fln2 = A * f5 + B * f4 + C * f3 + D * f2 + E * f + F;

				// Parameterize T from target arc-length.
				float fln = fln2;
				float ft = float(iln1);
				for (int32_t j = iln0; j < iln1; ++j)
				{
					if (fln < m_arcLengths[j + 1])
					{
						float aln0 = m_arcLengths[j];
						float aln1 = m_arcLengths[j + 1];
						//T_ASSERT (fln >= aln0 && fln <= aln1);
						ft = j + (fln - aln0) / (aln1 - aln0);
						break;
					}
				}

				Tln = ft / (m_arcLengths.size() - 1);
				//T_ASSERT (Tln >= 0.0f && Tln <= 1.0f);
				break;
			}
		}

		Frame frame = m_spline->evaluate(Scalar(Tln), Scalar(1.0f));
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

#include <limits>
#include "Core/Log/Log.h"
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

	void get(
		float& inoutT,
		float& outTension,
		float& outBias,
		float& outContinuity,
		TransformPath::Frame& outV0,
		TransformPath::Frame& outV1,
		TransformPath::Frame& outVp,
		TransformPath::Frame& outVn
	) const
	{
		int32_t nkeys = int32_t(m_keys.size());
		float T0, T1;

		int32_t index = 0;
		for (index = nkeys - 1; index > 0; --index)
		{
			if (inoutT >= m_keys[index].T)
				break;
		}

		T0 = m_keys[index].T;

		outV0 = m_keys[index].value;
		outTension = m_keys[index].tcb.x();
		outBias = m_keys[index].tcb.y();
		outContinuity = m_keys[index].tcb.z();

		int32_t index_1 = index + 1;
		if (index_1 < nkeys)
		{
			T1 = m_keys[index_1].T;
			outV1 = m_keys[index_1].value;
		}
		else
		{
			T1 = m_keys[nkeys - 1].T;
			outV1 = m_keys[nkeys - 1].value;
		}

		int32_t index_p = index - 1;
		if (index_p >= 0)
			outVp = m_keys[index_p].value;
		else
		{
			index_p = nkeys - 1;
			outVp = m_keys[0].value;
		}

		int32_t index_n = index_1 + 1;
		if (index_n < nkeys)
			outVn = m_keys[index_n].value;
		else
		{
			index_n = nkeys - 1;
			outVn = m_keys[nkeys - 1].value;
		}

		if (T0 < T1 - FUZZY_EPSILON)
			inoutT = (inoutT - T0) / (T1 - T0);
		else
			inoutT = 0.0f;
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
};

/*! \brief Closed uniform TCB spline accessor. */
class T_MATH_ALIGN16 ClosedUniformAccessor
{
public:
	ClosedUniformAccessor(const AlignedVector< TransformPath::Key >& keys, float Tend, float Tloop)
	:	m_keys(keys)
	,	m_Tend(Tend)
	,	m_Tloop(Tloop)
	,	m_Iloop(0)
	{
		int32_t nkeys = int32_t(m_keys.size());
		for (m_Iloop = nkeys - 1; m_Iloop >= 0; --m_Iloop)
		{
			if (m_Tloop > m_keys[m_Iloop].T + FUZZY_EPSILON)
				break;
		}
	}

	void get(
		float& inoutT,
		float& outTension,
		float& outBias,
		float& outContinuity,
		TransformPath::Frame& outV0,
		TransformPath::Frame& outV1,
		TransformPath::Frame& outVp,
		TransformPath::Frame& outVn
	) const
	{
		int32_t nkeys = int32_t(m_keys.size());
		float T0, T1;

		if (inoutT <= m_Tend)
		{
			// Initial lap

			int32_t index = 0;
			for (index = nkeys - 1; index >= 0; --index)
			{
				if (inoutT >= m_keys[index].T)
					break;
			}

			T0 = m_keys[index].T;

			outV0 = m_keys[index].value;
			outTension = m_keys[index].tcb.x();
			outBias = m_keys[index].tcb.y();
			outContinuity = m_keys[index].tcb.z();

			int32_t index_1 = index + 1;
			if (index_1 < nkeys)
			{
				T1 = m_keys[index_1].T;
				outV1 = m_keys[index_1].value;
			}
			else
			{
				index_1 = m_Iloop + 1;
				T1 = m_Tend + (m_keys[index_1].T - m_Tloop);
				outV1 = m_keys[index_1].value;
			}

			int32_t index_p = index - 1;
			if (index_p >= 0)
				outVp = m_keys[index_p].value;
			else
			{
				index_p = nkeys - 1;
				outVp = m_keys[0].value;
			}

			int32_t index_n = index_1 + 1;
			if (index_n < nkeys)
				outVn = m_keys[index_n].value;
			else
			{
				index_n = m_Iloop + 1;
				outVn = m_keys[index_n].value;
			}

			//log::info.setDecimals(2);
			//log::info << L"1: " << inoutT << L" " << T0 << L" -> " << T1 << L"  " << index_p << L", [" << index << L"], " << index_1 << L", " << index_n << L"   loop " << m_Iloop << Endl;

			inoutT = (inoutT - T0) / (T1 - T0);
		}
		else
		{
			// Loop lap

			inoutT -= m_Tend;

			while (inoutT > m_Tend - m_Tloop)
				inoutT -= m_Tend - m_Tloop;

			inoutT += m_Tloop;

			int32_t index = 0;
			for (index = nkeys - 1; index >= 0; --index)
			{
				if (inoutT >= m_keys[index].T)
					break;
			}

			if (index <= m_Iloop)
				index = nkeys - 1;

			T0 = m_keys[index].T;

			outV0 = m_keys[index].value;
			outTension = m_keys[index].tcb.x();
			outBias = m_keys[index].tcb.y();
			outContinuity = m_keys[index].tcb.z();

			int32_t index_1 = index + 1;
			if (index_1 < nkeys)
			{
				T1 = m_keys[index_1].T;
				outV1 = m_keys[index_1].value;
			}
			else
			{
				index_1 = m_Iloop + 1;
				T1 = m_Tend + (m_keys[index_1].T - m_Tloop);
				outV1 = m_keys[index_1].value;
			}

			int32_t index_p = index - 1;
			if (index_p > m_Iloop)
				outVp = m_keys[index_p].value;
			else
			{
				index_p = nkeys - 1;
				outVp = m_keys[nkeys - 1].value;
			}

			int32_t index_n = index_1 + 1;
			if (index_n < nkeys)
				outVn = m_keys[index_n].value;
			else
			{
				index_n = m_Iloop + 1;
				outVn = m_keys[index_n].value;
			}

			if (inoutT < T0)
				inoutT += m_Tend - m_Tloop;

			//log::info.setDecimals(2);
			//log::info << L"2: " << inoutT << L" " << T0 << L" -> " << T1 << L"  " << index_p << L", [" << index << L"], " << index_1 << L", " << index_n << L"   loop " << m_Iloop << Endl;

			inoutT = (inoutT - T0) / (T1 - T0);
		}
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
	float m_Tend;
	float m_Tloop;
	int32_t m_Iloop;
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

TransformPath::Frame TransformPath::evaluate(float at) const
{
	float Tend = getEndTime() + getStartTime();
	return evaluate(at, Tend);
}

TransformPath::Frame TransformPath::evaluate(float at, float end) const
{
	size_t nkeys = m_keys.size();
	if (nkeys == 0)
		return Frame();
	else if (nkeys == 1)
		return m_keys[0].value;
	else
	{
		if (!m_spline.ptr())
		{
			m_spline.reset(new TcbSpline< Key, Frame, OpenUniformAccessor >(
				OpenUniformAccessor(m_keys)
			));
		}

		Frame frame = m_spline->evaluate(at);
		frame.position = frame.position.xyz1();
		frame.orientation = frame.orientation.xyz0();

		return frame;
	}
}

TransformPath::Frame TransformPath::evaluate(float at, float end, float loop) const
{
	size_t nkeys = m_keys.size();
	if (nkeys == 0)
		return Frame();
	else if (nkeys == 1)
		return m_keys[0].value;
	else
	{
		if (!m_spline.ptr())
		{
			m_spline.reset(new TcbSpline< Key, Frame, ClosedUniformAccessor >(
				ClosedUniformAccessor(m_keys, end, loop)
			));
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

void TransformPath::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< Key, MemberComposite< Key > >(L"keys", m_keys);
}

void TransformPath::Key::serialize(ISerializer& s)
{
	s >> Member< float >(L"T", T, AttributeRange(0.0f));
	s >> Member< Vector4 >(L"tcb", tcb);
	s >> MemberComposite< Frame >(L"value", value);
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

void TransformPath::Frame::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"position", position, AttributePoint());
	s >> Member< Vector4 >(L"orientation", orientation, AttributeDirection());
}

}

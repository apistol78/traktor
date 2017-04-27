/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Hermite.h"
#include "Core/Math/Float.h"
#include "Core/Math/TcbSpline.h"
#include "Core/Math/TransformPath.h"
#include "Core/Serialization/AttributeAngles.h"
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

/*! \brief Key value wrapped into separate member to be compatible with legacy data. */
class MemberKeyValue : public MemberComplex  
{
public:
	MemberKeyValue(const wchar_t* const name, Vector4& refPosition, Vector4& refOrientation)
	:	MemberComplex(name, true)
	,	m_refPosition(refPosition)
	,	m_refOrientation(refOrientation)
	{
	}

	virtual void serialize(ISerializer& s) const T_OVERRIDE T_FINAL
	{
		s >> Member< Vector4 >(L"position", m_refPosition, AttributePoint());
		s >> Member< Vector4 >(L"orientation", m_refOrientation, AttributeAngles());
	}

private:
	Vector4& m_refPosition;
	Vector4& m_refOrientation;
};

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
		TransformPath::Key& outV0,
		TransformPath::Key& outV1,
		TransformPath::Key& outVp,
		TransformPath::Key& outVn
	) const
	{
		int32_t nkeys = int32_t(m_keys.size());
		float T0, T1;

		int32_t index = 0;
		for (index = nkeys - 1; index > 0; --index)
		{
			if (inoutT >= m_keys[index].T + FUZZY_EPSILON)
				break;
		}

		T0 = m_keys[index].T;

		outV0 = m_keys[index];

		int32_t index_1 = index + 1;
		if (index_1 < nkeys)
		{
			T1 = m_keys[index_1].T;
			outV1 = m_keys[index_1];
		}
		else
		{
			index_1 = nkeys - 1;
			T1 = m_keys[nkeys - 1].T;
			outV1 = m_keys[nkeys - 1];
		}

		int32_t index_p = index - 1;
		if (index_p >= 0)
			outVp = m_keys[index_p];
		else
		{
			index_p = nkeys - 1;
			outVp = m_keys[0];
		}

		int32_t index_n = index_1 + 1;
		if (index_n < nkeys)
			outVn = m_keys[index_n];
		else
		{
			index_n = nkeys - 1;
			outVn = m_keys[nkeys - 1];
		}

		if (T0 < T1 - FUZZY_EPSILON)
			inoutT = (inoutT - T0) / (T1 - T0);
		else
			inoutT = 0.0f;

		outTension = lerp(m_keys[index].tcb.x(), m_keys[index_1].tcb.x(), inoutT);
		outBias = lerp(m_keys[index].tcb.y(), m_keys[index].tcb.y(), inoutT);
		outContinuity = lerp(m_keys[index].tcb.z(), m_keys[index].tcb.z(), inoutT);
	}

	TransformPath::Key combine(
		float t,
		const TransformPath::Key& v0, float w0,
		const TransformPath::Key& v1, float w1,
		const TransformPath::Key& v2, float w2,
		const TransformPath::Key& v3, float w3
	) const
	{
		TransformPath::Key f;
		f.T = t;
		f.tcb = v0.tcb;
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
		TransformPath::Key& outV0,
		TransformPath::Key& outV1,
		TransformPath::Key& outVp,
		TransformPath::Key& outVn
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

			outV0 = m_keys[index];

			int32_t index_1 = index + 1;
			if (index_1 < nkeys)
			{
				T1 = m_keys[index_1].T;
				outV1 = m_keys[index_1];
			}
			else
			{
				index_1 = m_Iloop + 1;
				T1 = m_Tend + (m_keys[index_1].T - m_Tloop);
				outV1 = m_keys[index_1];
			}

			int32_t index_p = index - 1;
			if (index_p >= 0)
				outVp = m_keys[index_p];
			else
			{
				index_p = nkeys - 1;
				outVp = m_keys[0];
			}

			int32_t index_n = index_1 + 1;
			if (index_n < nkeys)
				outVn = m_keys[index_n];
			else
			{
				index_n = m_Iloop + 1;
				outVn = m_keys[index_n];
			}

			inoutT = (inoutT - T0) / (T1 - T0);

			outTension = lerp(m_keys[index].tcb.x(), m_keys[index_1].tcb.x(), inoutT);
			outBias = lerp(m_keys[index].tcb.y(), m_keys[index].tcb.y(), inoutT);
			outContinuity = lerp(m_keys[index].tcb.z(), m_keys[index].tcb.z(), inoutT);
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

			outV0 = m_keys[index];

			int32_t index_1 = index + 1;
			if (index_1 < nkeys)
			{
				T1 = m_keys[index_1].T;
				outV1 = m_keys[index_1];
			}
			else
			{
				index_1 = m_Iloop + 1;
				T1 = m_Tend + (m_keys[index_1].T - m_Tloop);
				outV1 = m_keys[index_1];
			}

			int32_t index_p = index - 1;
			if (index_p > m_Iloop)
				outVp = m_keys[index_p];
			else
			{
				index_p = nkeys - 1;
				outVp = m_keys[nkeys - 1];
			}

			int32_t index_n = index_1 + 1;
			if (index_n < nkeys)
				outVn = m_keys[index_n];
			else
			{
				index_n = m_Iloop + 1;
				outVn = m_keys[index_n];
			}

			if (inoutT < T0)
				inoutT += m_Tend - m_Tloop;

			inoutT = (inoutT - T0) / (T1 - T0);

			outTension = lerp(m_keys[index].tcb.x(), m_keys[index_1].tcb.x(), inoutT);
			outBias = lerp(m_keys[index].tcb.y(), m_keys[index].tcb.y(), inoutT);
			outContinuity = lerp(m_keys[index].tcb.z(), m_keys[index].tcb.z(), inoutT);
		}
	}

	TransformPath::Key combine(
		float t,
		const TransformPath::Key& v0, float w0,
		const TransformPath::Key& v1, float w1,
		const TransformPath::Key& v2, float w2,
		const TransformPath::Key& v3, float w3
	) const
	{
		TransformPath::Key f;
		f.T = t;
		f.tcb = v0.tcb;
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

size_t TransformPath::insert(const Key& key)
{
	AlignedVector< Key >::iterator at = m_keys.end();
	size_t keys = m_keys.size();
	if (keys >= 1)
	{
		if (key.T <= m_keys.front().T)
			at = m_keys.insert(m_keys.begin(), key);
		else if (key.T >= m_keys.back().T)
			at = m_keys.insert(m_keys.end(), key);
		else
		{
			for (size_t i = 0; i < keys - 1; ++i)
			{
				const Key& k1 = m_keys[i];
				const Key& k2 = m_keys[i + 1];
				if (key.T > k1.T && key.T < k2.T)
				{
					AlignedVector< Key >::iterator iter = m_keys.begin();
                    std::advance(iter, int32_t(i + 1));
					at = m_keys.insert(iter, key);
					break;
				}
			}
		}
	}
	else
	{
		m_keys.push_back(key);
		at = m_keys.end();
	}
	m_spline.release();
	return std::distance(m_keys.begin(), at);
}

TransformPath::Key TransformPath::evaluate(float at) const
{
	float Tend = getEndTime() + getStartTime();
	return evaluate(at, Tend);
}

TransformPath::Key TransformPath::evaluate(float at, float end) const
{
	size_t nkeys = m_keys.size();
	if (nkeys == 0)
		return Key();
	else if (nkeys == 1)
		return m_keys[0];
	else
	{
		if (!m_spline.ptr())
		{
			m_spline.reset(new TcbSpline< Key, Key, OpenUniformAccessor >(
				OpenUniformAccessor(m_keys)
			));
		}

		Key key = m_spline->evaluate(at);
		key.T = at;
		key.position = key.position.xyz1();
		key.orientation = key.orientation.xyz0();

		return key;
	}
}

TransformPath::Key TransformPath::evaluate(float at, float end, float loop) const
{
	size_t nkeys = m_keys.size();
	if (nkeys == 0)
		return Key();
	else if (nkeys == 1)
		return m_keys[0];
	else
	{
		if (!m_spline.ptr())
		{
			m_spline.reset(new TcbSpline< Key, Key, ClosedUniformAccessor >(
				ClosedUniformAccessor(m_keys, end, loop)
			));
		}

		Key key = m_spline->evaluate(at);
		key.T = at;
		key.position = key.position.xyz1();
		key.orientation = key.orientation.xyz0();

		return key;
	}
}

int32_t TransformPath::getClosestKey(float at) const
{
	if (m_keys.empty())
		return -1;

	float minT = std::numeric_limits< float >::max();
	int32_t minI = 0;

	for (int32_t i = 0; i < int32_t(m_keys.size()); ++i)
	{
		float dT = abs(at - m_keys[i].T);
		if (dT < minT)
		{
			minT = dT;
			minI = i;
		}
	}

	return minI;
}

int32_t TransformPath::getClosestPreviousKey(float at) const
{
	if (m_keys.empty())
		return -1;

	float minT = std::numeric_limits< float >::max();
	int32_t minI = -1;

	for (int32_t i = 0; i < int32_t(m_keys.size()); ++i)
	{
		if (at <= m_keys[i].T + FUZZY_EPSILON)
			continue;

		float dT = at - m_keys[i].T;
		if (dT < minT)
		{
			minT = dT;
			minI = i;
		}
	}

	return minI;
}

int32_t TransformPath::getClosestNextKey(float at) const
{
	if (m_keys.empty())
		return -1;

	float minT = std::numeric_limits< float >::max();
	int32_t minI = -1;

	for (int32_t i = 0; i < int32_t(m_keys.size()); ++i)
	{
		if (at >= m_keys[i].T - FUZZY_EPSILON)
			continue;

		float dT = m_keys[i].T - at;
		if (dT < minT)
		{
			minT = dT;
			minI = i;
		}
	}

	return minI;
}

void TransformPath::split(float at, TransformPath& outPath1, TransformPath& outPath2) const
{
	for (uint32_t i = 0; i < uint32_t(m_keys.size()); ++i)
	{
		if (m_keys[i].T < at)
			outPath1.m_keys.push_back(m_keys[i]);
		else
		{
			outPath2.m_keys.push_back(m_keys[i]);
			outPath2.m_keys.back().T -= at;
		}
	}

	Key f = evaluate(at);
	outPath1.insert(f);

	f.T = 0.0f;
	outPath2.insert(f);
}

void TransformPath::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< Key, MemberComposite< Key > >(L"keys", m_keys);
}

Transform TransformPath::Key::transform() const
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

void TransformPath::Key::serialize(ISerializer& s)
{
	s >> Member< float >(L"T", T, AttributeRange(0.0f));
	s >> Member< Vector4 >(L"tcb", tcb);
	s >> MemberKeyValue(L"value", position, orientation);
}

TransformPath& TransformPath::operator = (const TransformPath& path)
{
	m_keys = path.m_keys;
	return *this;
}

}

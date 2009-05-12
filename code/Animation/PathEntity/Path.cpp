#include <limits>
#include "Animation/PathEntity/Path.h"
#include "Core/Math/Hermite.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace animation
	{
		namespace
		{

struct FrameAccessor
{
	static inline Scalar time(const Path::Key& key)
	{
		return Scalar(key.T);
	}

	static inline Path::Frame value(const Path::Key& key)
	{
		return Path::Frame(key.value);
	}

	static inline Path::Frame combine(
		const Path::Frame& v0, const Scalar& w0,
		const Path::Frame& v1, const Scalar& w1,
		const Path::Frame& v2, const Scalar& w2,
		const Path::Frame& v3, const Scalar& w3
	)
	{
		Path::Frame f;
		f.position = v0.position * w0 + v1.position * w1 + v2.position * w2 + v3.position * w3;
		f.orientation = (v0.orientation * w0 + v1.orientation * w1 + v2.orientation * w2 + v3.orientation * w3).normalized();
		return f;
	}
};

		}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.animation.Path", Path, Serializable)

void Path::insert(float at, const Frame& frame)
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
}

Path::Frame Path::evaluate(float at) const
{
	if (m_keys.empty())
		return Frame();
	return Hermite< Key, Scalar, Frame, FrameAccessor >::evaluate(&m_keys[0], m_keys.size(), Scalar(at));
}

Path::Frame* Path::getClosestKeyFrame(float at)
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

	return &m_keys[minI].value;
}

bool Path::serialize(Serializer& s)
{
	return s >> MemberAlignedVector< Key, MemberComposite< Key > >(L"keys", m_keys);
}

bool Path::Key::serialize(Serializer& s)
{
	s >> Member< float >(L"T", T);
	s >> MemberComposite< Frame >(L"value", value);
	return true;
}

bool Path::Frame::serialize(Serializer& s)
{
	s >> Member< Vector4 >(L"position", position);
	s >> Member< Quaternion >(L"orientation", orientation);
	return true;
}

	}
}

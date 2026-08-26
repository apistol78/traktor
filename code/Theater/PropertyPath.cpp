/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Theater/PropertyPath.h"

#include "Core/Class/Any.h"
#include "Core/Class/Boxes/BoxedColor4f.h"
#include "Core/Class/Boxes/BoxedQuaternion.h"
#include "Core/Class/Boxes/BoxedTransform.h"
#include "Core/Class/Boxes/BoxedVector2.h"
#include "Core/Class/Boxes/BoxedVector4.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/TcbSpline.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Thread/Acquire.h"
#include "World/IEntityComponentData.h"

#include <limits>

namespace traktor::theater
{
namespace
{

/*! Open uniform TCB spline accessor; a spline is built for a single property at a time. */
class T_MATH_ALIGN16 OpenUniformAccessor
{
public:
	explicit OpenUniformAccessor(const AlignedVector< PropertyPath::SplineKey >& keys)
		: m_keys(keys)
	{
	}

	void get(
		float& inoutT,
		float& outTension,
		float& outBias,
		float& outContinuity,
		PropertyPath::SplineKey& outV0,
		PropertyPath::SplineKey& outV1,
		PropertyPath::SplineKey& outVp,
		PropertyPath::SplineKey& outVn) const
	{
		const int32_t nkeys = int32_t(m_keys.size());

		int32_t index = 0;
		for (index = nkeys - 1; index > 0; --index)
			if (inoutT >= m_keys[index].T + FUZZY_EPSILON)
				break;

		const float T0 = m_keys[index].T;
		float T1;

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

		const int32_t index_p = index - 1;
		outVp = (index_p >= 0) ? m_keys[index_p] : m_keys[0];

		const int32_t index_n = index_1 + 1;
		outVn = (index_n < nkeys) ? m_keys[index_n] : m_keys[nkeys - 1];

		if (T0 < T1 - FUZZY_EPSILON)
			inoutT = (inoutT - T0) / (T1 - T0);
		else
			inoutT = 0.0f;

		outTension = lerp(m_keys[index].tcb.x(), m_keys[index_1].tcb.x(), Scalar(inoutT));
		outBias = lerp(m_keys[index].tcb.y(), m_keys[index_1].tcb.y(), Scalar(inoutT));
		outContinuity = lerp(m_keys[index].tcb.z(), m_keys[index_1].tcb.z(), Scalar(inoutT));
	}

	PropertyPath::SplineKey combine(
		float t,
		const PropertyPath::SplineKey& v0, float w0,
		const PropertyPath::SplineKey& v1, float w1,
		const PropertyPath::SplineKey& v2, float w2,
		const PropertyPath::SplineKey& v3, float w3) const
	{
		PropertyPath::SplineKey f;

		f.T = t;
		f.tcb = v0.tcb;
		f.value.value = v0.value.value * Scalar(w0) + v1.value.value * Scalar(w1) + v2.value.value * Scalar(w2) + v3.value.value * Scalar(w3);
		f.value.orientation = v0.value.orientation * Scalar(w0) + v1.value.orientation * Scalar(w1) + v2.value.orientation * Scalar(w2) + v3.value.orientation * Scalar(w3);

		return f;
	}

private:
	const AlignedVector< PropertyPath::SplineKey >& m_keys;
};

/*! A path used to animate a single property, with a value of it per key. */
struct LegacyKey
{
	float T = 0.0f;
	Vector4 value = Vector4::zero();
	Vector4 orientation = Vector4::zero();
	Vector4 tcb = Vector4::zero();
	Ref< world::IEntityComponentData > componentData;

	void serialize(ISerializer& s)
	{
		s >> Member< float >(L"T", T);
		s >> Member< Vector4 >(L"value", value);
		s >> Member< Vector4 >(L"orientation", orientation);
		s >> Member< Vector4 >(L"tcb", tcb);

		if (s.getVersion< PropertyPath >() >= 1)
			s >> MemberRef< world::IEntityComponentData >(L"componentData", componentData);
	}
};

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.theater.PropertyPath", 2, PropertyPath, ISerializable)

const wchar_t* const PropertyPath::c_transformProperty = L"transform";

void PropertyPath::Property::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"componentType", componentType);
	s >> Member< std::wstring >(L"propertyName", propertyName);
}

Transform PropertyPath::Value::transform() const
{
	return Transform(
		value.xyz0(),
		Quaternion::fromEulerAngles(
			orientation.x(),
			orientation.y(),
			orientation.z()));
}

void PropertyPath::Value::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"value", value);
	s >> Member< Vector4 >(L"orientation", orientation);
}

void PropertyPath::Key::serialize(ISerializer& s)
{
	s >> Member< float >(L"T", T);
	s >> Member< Vector4 >(L"tcb", tcb);
	s >> MemberAlignedVector< Value, MemberComposite< Value > >(L"values", values);
	s >> MemberRefArray< world::IEntityComponentData >(L"componentData", componentData);
}

PropertyPath::ValueType PropertyPath::typeOfValue(const Any& value)
{
	// Integers most often represent discrete values which cannot be interpolated.
	if (value.isFloat() || value.isDouble())
		return ValueType::Scalar;
	else if (value.isObject< BoxedVector2 >())
		return ValueType::Vector2;
	else if (value.isObject< BoxedVector4 >())
		return ValueType::Vector4;
	else if (value.isObject< BoxedColor4f >())
		return ValueType::Color;
	else if (value.isObject< BoxedQuaternion >())
		return ValueType::Quaternion;
	else if (value.isObject< BoxedTransform >())
		return ValueType::Transform;
	else
		return ValueType::Invalid;
}

void PropertyPath::pack(const Any& value, Value& outValue)
{
	switch (typeOfValue(value))
	{
	case ValueType::Scalar:
		outValue.value = Vector4(value.getFloat(), 0.0f, 0.0f, 0.0f);
		break;

	case ValueType::Vector2:
		{
			const Vector2& v = CastAny< Vector2 >::get(value);
			outValue.value = Vector4(v.x, v.y, 0.0f, 0.0f);
		}
		break;

	case ValueType::Vector4:
		outValue.value = CastAny< Vector4 >::get(value);
		break;

	case ValueType::Color:
		outValue.value = CastAny< Color4f >::get(value);
		break;

	case ValueType::Quaternion:
		outValue.value = CastAny< Quaternion >::get(value).e;
		break;

	case ValueType::Transform:
		{
			const Transform& T = CastAny< Transform >::get(value);
			outValue.value = T.translation().xyz1();
			outValue.orientation = T.rotation().toEulerAngles();
		}
		break;

	default:
		break;
	}
}

Any PropertyPath::unpack(ValueType valueType, const Value& value)
{
	switch (valueType)
	{
	case ValueType::Scalar:
		return Any::fromFloat(value.value.x());

	case ValueType::Vector2:
		return CastAny< Vector2 >::set(Vector2(value.value.x(), value.value.y()));

	case ValueType::Vector4:
		return CastAny< Vector4 >::set(value.value);

	case ValueType::Color:
		return CastAny< Color4f >::set(Color4f(value.value));

	case ValueType::Quaternion:
		// Interpolated, and split, values are not necessarily of unit length.
		return CastAny< Quaternion >::set(Quaternion(value.value.normalized()));

	case ValueType::Transform:
		return CastAny< Transform >::set(value.transform());

	default:
		return Any();
	}
}

int32_t PropertyPath::addProperty(const std::wstring& componentType, const std::wstring& propertyName)
{
	const int32_t index = findProperty(componentType, propertyName);
	if (index >= 0)
		return index;

	Property& property = m_properties.push_back();
	property.componentType = componentType;
	property.propertyName = propertyName;

	// Every key hold a value of every property; the new one is zero until captured.
	for (auto& key : m_keys)
		key.values.resize(m_properties.size());

	flush();
	return (int32_t)m_properties.size() - 1;
}

int32_t PropertyPath::findProperty(const std::wstring& componentType, const std::wstring& propertyName) const
{
	for (int32_t i = 0; i < (int32_t)m_properties.size(); ++i)
		if (m_properties[i].componentType == componentType && m_properties[i].propertyName == propertyName)
			return i;
	return -1;
}

size_t PropertyPath::insert(const Key& key)
{
	auto at = m_keys.end();
	const size_t keys = m_keys.size();
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
				if (key.T > m_keys[i].T && key.T < m_keys[i + 1].T)
				{
					auto iter = m_keys.begin();
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
		at = m_keys.begin();
	}

	const size_t index = std::distance(m_keys.begin(), at);
	m_keys[index].values.resize(m_properties.size());

	flush();
	return index;
}

PropertyPath::Value PropertyPath::evaluate(int32_t property, ValueType valueType, float at) const
{
	const size_t nkeys = m_keys.size();
	if (nkeys == 0 || property < 0 || property >= (int32_t)m_properties.size())
		return Value();
	else if (nkeys == 1)
		return m_keys[0].values[property];

	if (valueType == ValueType::Transform)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

		// The spline is built for a single property; rebuild it as another is evaluated.
		if (!m_spline.ptr() || m_splineProperty != property)
		{
			m_splineKeys.resize(nkeys);
			for (size_t i = 0; i < nkeys; ++i)
			{
				m_splineKeys[i].T = m_keys[i].T;
				m_splineKeys[i].tcb = m_keys[i].tcb;
				m_splineKeys[i].value = m_keys[i].values[property];
			}

			m_spline.reset(new TcbSpline< SplineKey, SplineKey, OpenUniformAccessor >(
				OpenUniformAccessor(m_splineKeys)));
			m_splineProperty = property;
		}

		Value value = m_spline->evaluate(at).value;
		value.value = value.value.xyz1();
		value.orientation = value.orientation.xyz0();
		return value;
	}

	// Keys are not necessarily ordered in time; find those bracketing the time.
	const Key* k0 = nullptr;
	const Key* k1 = nullptr;
	for (const auto& key : m_keys)
	{
		if (key.T <= at && (k0 == nullptr || key.T > k0->T))
			k0 = &key;
		if (key.T >= at && (k1 == nullptr || key.T < k1->T))
			k1 = &key;
	}

	// Paths are never closed nor extrapolated; clamp at both ends.
	if (k0 == nullptr)
		return k1->values[property];
	if (k1 == nullptr)
		return k0->values[property];

	const float dT = k1->T - k0->T;
	if (dT <= FUZZY_EPSILON)
		return k0->values[property];

	const float T = (at - k0->T) / dT;

	Value value;
	if (valueType == ValueType::Quaternion)
		value.value = slerp(Quaternion(k0->values[property].value), Quaternion(k1->values[property].value), T).e;
	else
		value.value = lerp(k0->values[property].value, k1->values[property].value, Scalar(T));

	return value;
}

Any PropertyPath::evaluateValue(int32_t property, ValueType valueType, float at) const
{
	if (m_keys.empty())
		return Any();

	return unpack(valueType, evaluate(property, valueType, at));
}

int32_t PropertyPath::getClosestKey(float at) const
{
	if (m_keys.empty())
		return -1;

	float minT = std::numeric_limits< float >::max();
	int32_t minI = 0;

	for (int32_t i = 0; i < int32_t(m_keys.size()); ++i)
	{
		const float dT = traktor::abs(at - m_keys[i].T);
		if (dT < minT)
		{
			minT = dT;
			minI = i;
		}
	}

	return minI;
}

int32_t PropertyPath::getClosestPreviousKey(float at) const
{
	int32_t index = -1;
	for (int32_t i = 0; i < int32_t(m_keys.size()); ++i)
	{
		if (m_keys[i].T >= at - FUZZY_EPSILON)
			continue;
		if (index < 0 || m_keys[i].T > m_keys[index].T)
			index = i;
	}
	return index;
}

int32_t PropertyPath::getClosestNextKey(float at) const
{
	int32_t index = -1;
	for (int32_t i = 0; i < int32_t(m_keys.size()); ++i)
	{
		if (m_keys[i].T <= at + FUZZY_EPSILON)
			continue;
		if (index < 0 || m_keys[i].T < m_keys[index].T)
			index = i;
	}
	return index;
}

void PropertyPath::split(float at, PropertyPath& outPath1, PropertyPath& outPath2) const
{
	outPath1.m_properties = m_properties;
	outPath2.m_properties = m_properties;
	outPath1.m_keys.resize(0);
	outPath2.m_keys.resize(0);
	outPath1.flush();
	outPath2.flush();

	if (m_keys.empty())
		return;

	for (const auto& key : m_keys)
	{
		if (key.T < at)
			outPath1.m_keys.push_back(key);
		else
		{
			outPath2.m_keys.push_back(key);
			outPath2.m_keys.back().T -= at;
		}
	}

	// A key at the split point is added to both paths, its values interpolated linearly.
	Key k;
	k.T = at;
	k.values.resize(m_properties.size());
	for (int32_t i = 0; i < (int32_t)m_properties.size(); ++i)
		k.values[i] = evaluate(i, ValueType::Vector4, at);

	outPath1.insert(k);

	k.T = 0.0f;
	outPath2.insert(k);
}

Ref< PropertyPath > PropertyPath::stripped() const
{
	Ref< PropertyPath > path = new PropertyPath();

	path->m_properties = m_properties;
	path->m_keys = m_keys;

	// Only the editor look at the data of the components.
	for (auto& key : path->m_keys)
		key.componentData.resize(0);

	return path;
}

void PropertyPath::set(size_t at, const Key& k)
{
	m_keys[at] = k;
	m_keys[at].values.resize(m_properties.size());
	flush();
}

AlignedVector< PropertyPath::Key >& PropertyPath::editKeys()
{
	flush();
	return m_keys;
}

void PropertyPath::serialize(ISerializer& s)
{
	if (s.getVersion< PropertyPath >() >= 2)
	{
		s >> MemberAlignedVector< Property, MemberComposite< Property > >(L"properties", m_properties);
		s >> MemberAlignedVector< Key, MemberComposite< Key > >(L"keys", m_keys);
	}
	else
	{
		// A path used to animate a single property; the track merge such paths.
		Property& property = m_properties.push_back();
		s >> Member< std::wstring >(L"componentType", property.componentType);
		s >> Member< std::wstring >(L"propertyName", property.propertyName);

		AlignedVector< LegacyKey > legacyKeys;
		s >> MemberAlignedVector< LegacyKey, MemberComposite< LegacyKey > >(L"keys", legacyKeys);

		m_keys.resize(legacyKeys.size());
		for (size_t i = 0; i < legacyKeys.size(); ++i)
		{
			m_keys[i].T = legacyKeys[i].T;
			m_keys[i].tcb = legacyKeys[i].tcb;
			m_keys[i].values.resize(1);
			m_keys[i].values[0].value = legacyKeys[i].value;
			m_keys[i].values[0].orientation = legacyKeys[i].orientation;

			if (legacyKeys[i].componentData)
				m_keys[i].componentData.push_back(legacyKeys[i].componentData);
		}
	}

	if (s.getDirection() == ISerializer::Direction::Read)
	{
		for (auto& key : m_keys)
			key.values.resize(m_properties.size());

		flush();
	}
}

void PropertyPath::flush() const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_spline.release();
	m_splineProperty = -1;
}

}

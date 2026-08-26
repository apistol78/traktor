/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/ISpline.h"
#include "Core/Math/Transform.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Thread/Semaphore.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_THEATER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Any;

}

namespace traktor::world
{

class IEntityComponentData;

}

namespace traktor::theater
{

/*! Path of animated properties.
 * \ingroup Theater
 *
 * A path animate a set of properties which are captured, and keyed, together;
 * every key hold a value of each of them.
 */
class T_DLLCLASS PropertyPath : public ISerializable
{
	T_RTTI_CLASS;

public:
	/*! Type of property value; only interpolateable types are supported. */
	enum class ValueType
	{
		Invalid = 0,
		Scalar = 1,
		Vector2 = 2,
		Vector4 = 3,
		Color = 4,
		Quaternion = 5,
		Transform = 6
	};

	/*! Property animated by the path. */
	struct T_DLLCLASS Property
	{
		std::wstring componentType;
		std::wstring propertyName;

		/*! Check if the property is of the entity itself rather than of a component. */
		bool isEntityProperty() const { return componentType.empty(); }

		void serialize(ISerializer& s);
	};

	/*! Value of a single property at a key. */
	struct T_DLLCLASS Value
	{
		Vector4 value = Vector4::zero();
		Vector4 orientation = Vector4::zero();

		/*! Get value as a transform; only a transform use the orientation. */
		Transform transform() const;

		void serialize(ISerializer& s);
	};

	/*! Key frame; holds a value of every property animated by the path. */
	struct T_DLLCLASS Key
	{
		float T = 0.0f;
		Vector4 tcb = Vector4::zero();
		AlignedVector< Value > values;

		/*! Data of the components as they were captured; empty in a built scene. */
		RefArray< world::IEntityComponentData > componentData;

		void serialize(ISerializer& s);
	};

	/*! Name of the property through which the transform of an entity is animated. */
	static const wchar_t* const c_transformProperty;

	/*! Determine type of a value as read from a property; invalid if it cannot be interpolated. */
	static ValueType typeOfValue(const Any& value);

	/*! Pack a value, as read from a property, into the value of a key. */
	static void pack(const Any& value, Value& outValue);

	/*! Unpack the value of a key into a value which can be written to a property. */
	static Any unpack(ValueType valueType, const Value& value);

	/*! Add a property to those animated by the path; return its index. */
	int32_t addProperty(const std::wstring& componentType, const std::wstring& propertyName);

	/*! Get index of an animated property; -1 if the path doesn't animate it. */
	int32_t findProperty(const std::wstring& componentType, const std::wstring& propertyName) const;

	/*! Get properties animated by the path. */
	const AlignedVector< Property >& getProperties() const { return m_properties; }

	/*! Insert key into path; return index where it was inserted. */
	size_t insert(const Key& key);

	/*! Evaluate the value of a property; transforms follow a TCB spline, all other values are interpolated linearly. */
	Value evaluate(int32_t property, ValueType valueType, float at) const;

	/*! Evaluate a value, ready to be written to a property; void if the path is empty. */
	Any evaluateValue(int32_t property, ValueType valueType, float at) const;

	/*! Get closest key frame from time; -1 if the path has none. */
	int32_t getClosestKey(float at) const;

	/*! Get closest previous key frame from time. */
	int32_t getClosestPreviousKey(float at) const;

	/*! Get closest next key frame from time. */
	int32_t getClosestNextKey(float at) const;

	/*! Split path into two paths at given time. */
	void split(float at, PropertyPath& outPath1, PropertyPath& outPath2) const;

	/*! Get a copy of this path, without the component data only the editor need. */
	Ref< PropertyPath > stripped() const;

	bool empty() const { return m_keys.empty(); }

	size_t size() const { return m_keys.size(); }

	const Key& get(size_t at) const { return m_keys[at]; }

	void set(size_t at, const Key& k);

	const AlignedVector< Key >& keys() const { return m_keys; }

	AlignedVector< Key >& editKeys();

	virtual void serialize(ISerializer& s) override final;

	/*! Key of a single property; public only so the spline accessor can reach it. */
	struct SplineKey
	{
		float T = 0.0f;
		Vector4 tcb = Vector4::zero();
		Value value;
	};

private:
	AlignedVector< Property > m_properties;
	AlignedVector< Key > m_keys;
	mutable Semaphore m_lock;
	mutable AlignedVector< SplineKey > m_splineKeys;
	mutable AutoPtr< ISpline< SplineKey > > m_spline;
	mutable int32_t m_splineProperty = -1;

	void flush() const;
};

}

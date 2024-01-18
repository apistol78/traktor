/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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

/*! Transformation path.
 * \ingroup Core
 */
class T_DLLCLASS TransformPath : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct T_DLLCLASS Key
	{
		float T = 0.0f;
		Vector4 tcb = Vector4::zero();
		Vector4 position = Vector4::origo();
		Vector4 orientation = Vector4::zero();
		float values[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

		Transform transform() const;

		void serialize(ISerializer& s);
	};

	TransformPath() = default;

	TransformPath(const TransformPath& path);

	/*! Insert key into path.
	 *
	 * \param key New key frame.
	 * \return Index where key was inserted.
	 */
	size_t insert(const Key& key);

	/*! Evaluate frame.
	 *
	 * \param at Time to evaluate.
	 * \param closed Closed path.
	 * \return Evaluated frame.
	 */
	Key evaluate(float at, bool closed) const;

	/*! Get closest key frame from time.
	 *
	 * \param at Closest to time.
	 * \return Index of closest key frame, -1 if no key frame found.
	 */
	int32_t getClosestKey(float at) const;

	/*! Get closest previous key frame from time.
	 *
	 * \param at Closest to time.
	 * \return Index of closest key frame, -1 if no key frame found.
	 */
	int32_t getClosestPreviousKey(float at) const;

	/*! Get closest next key frame from time.
	 *
	 * \param at Closest to time.
	 * \return Index of closest key frame, -1 if no key frame found.
	 */
	int32_t getClosestNextKey(float at) const;

	/*!
	 */
	float measureLength(bool closed) const;

	/*!
	 */
	float measureSegmentLength(float from, float to, bool closed, int32_t steps = 1000) const;

	/*!
	 */
	float estimateTimeFromDistance(bool closed, float distance, int32_t steps = 1000) const;

	/*!
	 */
	TransformPath geometricNormalized(bool closed) const;

	/*! Split path into two paths at given time.
	 *
	 * \param at Split at time.
	 * \param outPath1 Path before split point.
	 * \param outPath2 Path after split point.
	 */
	void split(float at, TransformPath& outPath1, TransformPath& outPath2) const;

	/*! Get time of first key frame.
	 *
	 * \return Path's first key frame time.
	 */
	float getStartTime() const { return !m_keys.empty() ? m_keys.front().T : 0.0f; }

	/*! Get time of last key frame.
	 *
	 * \return Path's last key frame time.
	 */
	float getEndTime() const { return !m_keys.empty() ? m_keys.back().T : 0.0f; }

	/*! Get number of key frames.
	 *
	 * \return Number of key frames.
	 */
	size_t size() const { return m_keys.size(); }

	/*! Get key frame at index.
	 *
	 * \param at Index.
	 * \return Key frame at index.
	 */
	const Key& get(size_t at) const { return m_keys[at]; }

	/*!
	 */
	void set(size_t at, const Key& k);

	/*! Get key frames.
	 *
	 * \return Key frames.
	 */
	const AlignedVector< Key >& keys() const { return m_keys; }

	/*!
	 */
	AlignedVector< Key >& editKeys();

	/*!
	 */
	TransformPath& operator = (const TransformPath& path);

	/*!
	 */
	virtual void serialize(ISerializer& s) override final;

private:
	AlignedVector< Key > m_keys;
	mutable AutoPtr< ISpline< Key > > m_spline;
	mutable bool m_closed = false;
	mutable float m_length = -1.0f;

	void flush() const;
};

}


/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Math/Transform.h"
#include "Core/Math/Vector2.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::animation
{

/*! Skeleton bone.
 * \ingroup Animation
 */
class T_DLLCLASS Joint : public ISerializable
{
	T_RTTI_CLASS;

public:
	Joint();

	/*! Set parent joint index.
	 *
	 * \param parent New parent index.
	 */
	void setParent(int32_t parent) { m_parent = parent; }

	/*! Get parent joint index.
	 *
	 * \return Parent index.
	 */
	int32_t getParent() const { return m_parent; }

	/*! Set joint name.
	 *
	 * \param name New joint name.
	 */
	void setName(const std::wstring& name) { m_name = name; }

	/*! Get joint name.
	 *
	 * \return Name of joint.
	 */
	const std::wstring& getName() const { return m_name; }

	/*! Set joint transform.
	 *
	 * Joint transform is relative to parent space.
	 *
	 * \param transform New joint transform.
	 */
	void setTransform(const Transform& transform) { m_transform = transform; }

	/*! Get joint transform.
	 *
	 * Joint transform is relative to parent space.
	 *
	 * \return Joint transform.
	 */
	const Transform& getTransform() const { return m_transform; }

	/*! Set joint radius.
	 *
	 * \param radius New joint radius.
	 */
	void setRadius(float radius) { m_radius = radius; }

	/*! Get joint radius.
	 *
	 * \return Joint radius.
	 */
	const float getRadius() const { return m_radius; }

	virtual void serialize(ISerializer& s) override final;

private:
	int32_t m_parent;
	std::wstring m_name;
	Transform m_transform;
	float m_radius;
};

}

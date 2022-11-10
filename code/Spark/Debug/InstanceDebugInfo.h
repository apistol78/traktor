/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Aabb2.h"
#include "Core/Serialization/ISerializable.h"
#include "Spark/ColorTransform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

class CharacterInstance;

/*!
 * \ingroup Spark
 */
class T_DLLCLASS InstanceDebugInfo : public ISerializable
{
	T_RTTI_CLASS;

public:
	InstanceDebugInfo() = default;

	const std::string& getName() const { return m_name; }

	const Aabb2& getBounds() const { return m_bounds; }

	const Matrix33& getLocalTransform() const { return m_localTransform; }

	const Matrix33& getGlobalTransform() const { return m_globalTransform; }

	const ColorTransform& getColorTransform() const { return m_cxform; }

	bool getVisible() const { return m_visible; }

	virtual void serialize(ISerializer& s) override;

protected:
	std::string m_name;
	Aabb2 m_bounds;
	Matrix33 m_localTransform = Matrix33::identity();
	Matrix33 m_globalTransform = Matrix33::identity();
	ColorTransform m_cxform;
	bool m_visible = false;
};

	}
}

/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Quaternion.h"
#include "Core/Math/Transform.h"
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::scene
{

class EntityAdapter;

/*! Preview render camera. */
class T_DLLCLASS Camera : public Object
{
public:
	Camera();

	void setEnable(bool enable);

	bool isEnable() const;

	void place(const Vector4& position);

	void move(const Vector4& direction);

	void rotate(float dy, float dx);

	bool update(float deltaTime);

	Transform getWorld() const;

	Transform getView() const;

	void setPosition(const Vector4& position);

	const Vector4& getPosition() const { return m_position; }

	void setOrientation(const Quaternion& orientation);

	const Quaternion& getOrientation() const { return m_orientation; }

private:
	bool m_enable;
	Vector4 m_position;
	Quaternion m_orientation;
	Vector4 m_filteredPosition;
	Quaternion m_filteredOrientation;
};

}

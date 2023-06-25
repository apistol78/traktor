/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Math/Transform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Instance;

	}

	namespace model
	{

class Model;

	}

	namespace shape
	{

class T_DLLCLASS TracerCamera : public Object
{
	T_RTTI_CLASS;

public:
	explicit TracerCamera(
		const Transform& transform,
		float fieldOfView,
		int32_t width,
		int32_t height
	);

	const Transform& getTransform() const { return m_transform; }

	float getFieldOfView() const { return m_fieldOfView; }

	int32_t getWidth() const { return m_width; }

	int32_t getHeight() const { return m_height; }

private:
	Transform m_transform;
	float m_fieldOfView;
	int32_t m_width;
	int32_t m_height;
};

	}
}
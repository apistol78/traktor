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
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Aabb3.h"

namespace traktor
{
	namespace animation
	{

/*! \brief
 * \ingroup Animation
 */
class VolumePicker : public Object
{
	T_RTTI_CLASS;

public:
	void setPerspectiveTransform(const Matrix44& perspectiveTransform);

	void setViewTransform(const Matrix44& viewTransform);

	void removeAllVolumes();

	void addVolume(const Matrix44& worldTransform, const Aabb3& volume, int id);

	int traceVolume(const Vector4& clipPosition) const;

private:
	struct Volume
	{
		Matrix44 worldTransform;
		Aabb3 volume;
		int id;
	};

	Matrix44 m_perspectiveTransform;
	Matrix44 m_viewTransform;
	AlignedVector< Volume > m_volumes;
};

	}
}


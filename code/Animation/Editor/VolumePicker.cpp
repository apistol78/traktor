/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Animation/Editor/VolumePicker.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.VolumePicker", VolumePicker, Object)

void VolumePicker::setViewTransform(const Matrix44& viewTransform)
{
	m_viewTransform = viewTransform;
}

void VolumePicker::setPerspectiveTransform(const Matrix44& perspectiveTransform)
{
	m_perspectiveTransform = perspectiveTransform;
}

void VolumePicker::removeAllVolumes()
{
	m_volumes.resize(0);
}

void VolumePicker::addVolume(const Matrix44& worldTransform, const Aabb3& volume, int id)
{
	Volume v = { worldTransform, volume, id };
	m_volumes.push_back(v);
}

int VolumePicker::traceVolume(const Vector4& clipPosition) const
{
	Vector4 origin = m_perspectiveTransform.inverse() * clipPosition;
	Vector4 direction = m_perspectiveTransform.inverse() * Vector4(0.0f, 0.0f, 1.0f, 0.0f);

	origin = m_viewTransform.inverse() * origin;
	direction = m_viewTransform.inverse() * direction;

	float distance = std::numeric_limits< float >::max();
	int id = -1;

	for (AlignedVector< Volume >::const_iterator i = m_volumes.begin(); i != m_volumes.end(); ++i)
	{
		const Vector4 localOrigin = i->worldTransform.inverse() * origin;
		const Vector4 localDirection = i->worldTransform.inverse() * direction;
		Scalar distance;

		if (i->volume.intersectSegment(localOrigin, localDirection.normalized() * Scalar(1000.0f), distance))
		{
			const Scalar volumeDistance = (i->volume.getCenter() - localOrigin).length();
			if (volumeDistance < distance)
			{
				distance = volumeDistance;
				id = i->id;
			}
		}
	}

	return id;
}

}

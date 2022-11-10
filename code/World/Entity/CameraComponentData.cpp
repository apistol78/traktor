/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberEnum.h"
#include "World/Entity/CameraComponentData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.CameraComponentData", 0, CameraComponentData, IEntityComponentData)

CameraComponentData::CameraComponentData()
:	m_projection(Projection::Orthographic)
,	m_fov(deg2rad(70.0f))
,	m_width(1.0f)
,	m_height(1.0f)
{
}

void CameraComponentData::setProjection(Projection projection)
{
	m_projection = projection;
}

Projection CameraComponentData::getProjection() const
{
	return m_projection;
}

void CameraComponentData::setFieldOfView(float fov)
{
	m_fov = fov;
}

float CameraComponentData::getFieldOfView() const
{
	return m_fov;
}

void CameraComponentData::setWidth(float width)
{
	m_width = width;
}

float CameraComponentData::getWidth() const
{
	return m_width;
}

void CameraComponentData::setHeight(float height)
{
	m_height = height;
}

float CameraComponentData::getHeight() const
{
	return m_height;
}

void CameraComponentData::setTransform(const EntityData* owner, const Transform& transform)
{
}

void CameraComponentData::serialize(ISerializer& s)
{
	const MemberEnum< Projection >::Key kProjection[] =
	{
		{ L"Orthographic", Projection::Orthographic },
		{ L"Perspective", Projection::Perspective },
		{ 0 }
	};

	s >> MemberEnum< Projection >(L"projection", m_projection, kProjection);
	s >> Member< float >(L"fov", m_fov, AttributeUnit(UnitType::Radians));
	s >> Member< float >(L"width", m_width);
	s >> Member< float >(L"height", m_height);
}

	}
}

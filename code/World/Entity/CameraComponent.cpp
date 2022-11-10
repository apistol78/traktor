/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity/CameraComponent.h"
#include "World/Entity/CameraComponentData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.CameraComponent", CameraComponent, IEntityComponent)

CameraComponent::CameraComponent(const CameraComponentData* cameraData)
:	m_projection(cameraData->getProjection())
,	m_fov(cameraData->getFieldOfView())
,	m_width(cameraData->getWidth())
,	m_height(cameraData->getHeight())
{
}

void CameraComponent::destroy()
{
}

void CameraComponent::setOwner(Entity* owner)
{
}

void CameraComponent::update(const UpdateParams& update)
{
}

void CameraComponent::setTransform(const Transform& transform)
{
}

Aabb3 CameraComponent::getBoundingBox() const
{
	return Aabb3();
}

void CameraComponent::setProjection(Projection projection)
{
	m_projection = projection;
}

Projection CameraComponent::getProjection() const
{
	return m_projection;
}

void CameraComponent::setFieldOfView(float fov)
{
	m_fov = fov;
}

float CameraComponent::getFieldOfView() const
{
	return m_fov;
}

void CameraComponent::setWidth(float width)
{
	m_width = width;
}

float CameraComponent::getWidth() const
{
	return m_width;
}

void CameraComponent::setHeight(float height)
{
	m_height = height;
}

float CameraComponent::getHeight() const
{
	return m_height;
}

	}
}

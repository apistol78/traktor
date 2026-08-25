/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Transform.h"
#include "Resource/Proxy.h"
#include "World/IEntityComponent.h"

namespace traktor::render
{

class Shader;

}

namespace traktor::world
{

/*! Billboard component.
 * \ingroup World
 *
 * Draw a single quad textured from one tile of a pre-rendered atlas, standing in
 * for a detailed entity at distance.
 *
 * The atlas tiles are enumerated once, up front, as the orientation each of them
 * was rendered from; picking a tile is then a matter of finding the orientation
 * closest to how the entity is being looked at. Orienting the quad along that same
 * orientation, rather than straight at the camera, keep the quad in the plane the
 * tile was rasterized in, which is what stop a billboard seen from above from
 * turning edge on.
 */
class T_DLLCLASS BillboardComponent : public IEntityComponent
{
	T_RTTI_CLASS;

public:
	/*! A single atlas tile, and the orientation it was rendered from. */
	struct View
	{
		Vector4 right;    //!< Rightwards axis of the view, in the entity's own space.
		Vector4 up;       //!< Upwards axis of the view.
		Vector4 forward;  //!< Direction the view was rendered along, i.e. away from its camera.
		Vector4 texCoord; //!< Tile rectangle in the atlas as (u0, v0, u1, v1).
	};

	explicit BillboardComponent(
		const resource::Proxy< render::Shader >& shader,
		int32_t angles,
		int32_t elevations,
		float elevationFrom,
		float elevationTo,
		float size,
		const Vector4& offset,
		float startDistance,
		float cullDistance);

	virtual void destroy() override final;

	virtual void setOwner(Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const UpdateParams& update) override final;

	/*! Find the tile rendered closest to a direction.
	 *
	 * \param direction Direction being looked along, i.e. from the camera towards the
	 *                  billboard, expressed in the entity's own space. Need not be
	 *                  normalized.
	 */
	const View& findView(const Vector4& direction) const;

	const resource::Proxy< render::Shader >& getShader() const { return m_shader; }

	const Transform& getTransform() const { return m_transform; }

	float getSize() const { return m_size; }

	const Vector4& getOffset() const { return m_offset; }

	float getStartDistance() const { return m_startDistance; }

	float getCullDistance() const { return m_cullDistance; }

private:
	resource::Proxy< render::Shader > m_shader;
	AlignedVector< View > m_views;
	Transform m_transform = Transform::identity();
	float m_size;
	Vector4 m_offset;
	float m_startDistance;
	float m_cullDistance;
};

}

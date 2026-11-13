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

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class ITexture;
class Shader;

}

namespace traktor::world
{

/*! Billboard component.
 * \ingroup World
 *
 * Quad textured from the atlas tile rendered closest to the view direction,
 * oriented along the view the tile was rendered from.
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
		const resource::Proxy< render::ITexture >& albedoTexture,
		const resource::Proxy< render::ITexture >& normalTexture,
		int32_t angles,
		int32_t elevations,
		float elevationFrom,
		float elevationTo,
		float size,
		const Vector4& offset);

	virtual void destroy() override final;

	virtual void setOwner(Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const UpdateParams& update) override final;

	/*! Find the tile rendered closest to a direction, given in the entity's own space. */
	const View& findView(const Vector4& direction) const;

	const resource::Proxy< render::Shader >& getShader() const { return m_shader; }

	const resource::Proxy< render::ITexture >& getAlbedoTexture() const { return m_albedoTexture; }

	const resource::Proxy< render::ITexture >& getNormalTexture() const { return m_normalTexture; }

	const Transform& getTransform() const { return m_transform; }

	float getSize() const { return m_size; }

	const Vector4& getOffset() const { return m_offset; }

private:
	resource::Proxy< render::Shader > m_shader;
	resource::Proxy< render::ITexture > m_albedoTexture;
	resource::Proxy< render::ITexture > m_normalTexture;
	AlignedVector< View > m_views;
	Transform m_transform = Transform::identity();
	float m_size;
	Vector4 m_offset;
};

}

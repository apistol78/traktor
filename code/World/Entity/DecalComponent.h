/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Float.h"
#include "Resource/Proxy.h"
#include "World/IEntityComponent.h"

namespace traktor::render
{

class Shader;

}

namespace traktor::world
{

/*! Decal component.
 * \ingroup World
 */
class T_DLLCLASS DecalComponent : public IEntityComponent
{
	T_RTTI_CLASS;

public:
	explicit DecalComponent(
		const Vector2& size,
		float thickness,
		float alpha,
		float cullDistance,
		const resource::Proxy< render::Shader >& shader
	);

	virtual void destroy() override final;

	virtual void setOwner(Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const UpdateParams& update) override final;

	const Vector2& getSize() const { return m_size; }

	float getThickness() const { return m_thickness; }

	const resource::Proxy< render::Shader >& getShader() const { return m_shader; }

	float getAlpha() const { return m_alpha; }

	float getCullDistance() const { return m_cullDistance; }

	float getAge() const { return m_age; }

	const Transform& getTransform() const { return m_transform; }

private:
	Transform m_transform;
	Vector2 m_size;
	float m_thickness;
	float m_alpha;
	float m_cullDistance;
	float m_age;
	resource::Proxy< render::Shader > m_shader;
};

}

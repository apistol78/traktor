/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Vector2.h"
#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class Shader;

}

namespace traktor::world
{

/*! Decal component data.
 * \ingroup World
 */
class T_DLLCLASS DecalComponentData : public IEntityComponentData
{
	T_RTTI_CLASS;

public:
	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	const Vector2& getSize() const { return m_size; }

	float getThickness() const { return m_thickness; }

	float getAlpha() const { return m_alpha; }

	float getCullDistance() const { return m_cullDistance; }

	const resource::Id< render::Shader >& getShader() const { return m_shader; }

private:
	Vector2 m_size = Vector2(1.0f, 1.0f);
	float m_thickness = 1.0f;
	float m_alpha = 1.0f;
	float m_cullDistance = 100.0f;
	resource::Id< render::Shader > m_shader;
};

}

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
#include "Resource/Proxy.h"
#include "World/IEntityEvent.h"

namespace traktor::render
{

class Shader;

}

namespace traktor::world
{

/*!
 * \ingroup World
 */
class DecalEvent : public IEntityEvent
{
	T_RTTI_CLASS;

public:
	virtual Ref< IEntityEventInstance > createInstance(EventManagerComponent* eventManager, Entity* sender, const Transform& Toffset) const override final;

	const Vector2& getSize() const { return m_size; }

	float getThickness() const { return m_thickness; }

	float getAlpha() const { return m_alpha; }

	float getCullDistance() const { return m_cullDistance; }

	const resource::Proxy< render::Shader >& getShader() const { return m_shader; }

private:
	friend class WorldEntityFactory;

	Vector2 m_size = Vector2(1.0f, 1.0f);
	float m_thickness = 1.0f;
	float m_alpha = 2.0f;
	float m_cullDistance = 100.0f;
	resource::Proxy< render::Shader > m_shader;
};

}

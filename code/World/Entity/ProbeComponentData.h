/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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

class ITexture;

}

namespace traktor::world
{

/*! Reflection probe component.
 * \ingroup World
 *
 * \note A reflection probe is considered local if the
 * owner entity also contain a VolumeComponent.
 */
class T_DLLCLASS ProbeComponentData : public IEntityComponentData
{
	T_RTTI_CLASS;

public:
	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	void setTexture(const resource::Id< render::ITexture >& texture) { m_texture = texture; }

	const resource::Id< render::ITexture >& getTexture() const { return m_texture; }

	void setIntensity(float intensity) { m_intensity = intensity; }

	float getIntensity() const { return m_intensity; }

private:
	resource::Id< render::ITexture > m_texture;
	float m_intensity = 1.0f;
};

}

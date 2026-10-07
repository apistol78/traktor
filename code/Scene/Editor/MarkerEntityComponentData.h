/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
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
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class ITexture;

}

namespace traktor::scene
{

/*! Editor-only marker component.
 * \ingroup Scene
 *
 * Draws an icon at the owner entity in the scene editor; stripped when built.
 */
class T_DLLCLASS MarkerEntityComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< render::ITexture >& getTexture() const { return m_texture; }

	float getOffset() const { return m_offset; }

	float getSize() const { return m_size; }

private:
	resource::Id< render::ITexture > m_texture;
	float m_offset = 0.0f;
	float m_size = 0.5f;
};

}

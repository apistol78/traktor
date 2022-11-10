/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vector>
#include "Core/Ref.h"
#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IRenderSystem;
class Shader;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace animation
	{

class ClothComponent;

/*!
 * \ingroup Animation
 */
class T_DLLCLASS ClothComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	struct Anchor
	{
		uint32_t x;
		uint32_t y;

		void serialize(ISerializer& s);
	};

	ClothComponentData();

	Ref< ClothComponent > createComponent(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem
	) const;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< render::Shader >& getShader() const { return m_shader; }

private:
	resource::Id< render::Shader > m_shader;
	uint32_t m_resolutionX;
	uint32_t m_resolutionY;
	float m_scale;
	std::vector< Anchor > m_anchors;
	uint32_t m_solverIterations;
	float m_damping;
};

	}
}


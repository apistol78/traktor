/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Animation/PathEntity/PathComponent.h"
#include "Core/Ref.h"
#include "World/IEntityComponentData.h"

#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

class PathComponent;

/*!
 * \ingroup Animation
 */
class T_DLLCLASS PathComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	Ref< PathComponent > createComponent() const;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

private:
	TransformPath m_path;
	PathComponent::TimeMode m_timeMode = PathComponent::TimeMode::Manual;
	float m_timeOffset = 0.0f;
};

	}
}


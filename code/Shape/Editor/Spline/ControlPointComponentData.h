/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::shape
{

class ControlPointComponent;

/*!
 * \ingroup Shape
 */
class T_DLLCLASS ControlPointComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	Ref< ControlPointComponent > createComponent() const;

	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	float getScale() const { return m_scale; }

	float getAutomaticOrientationWeight() const { return m_automaticOrientationWeight; }

private:
	float m_scale = 1.0f;
	float m_automaticOrientationWeight = 1.0f;
};

}

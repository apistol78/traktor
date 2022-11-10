/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Shape/Editor/Spline/SplineLayerComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace shape
	{

class ExtrudeShapeLayerData;

/*!
 * \ingroup Shape
 */
class T_DLLCLASS ExtrudeShapeLayer : public SplineLayerComponent
{
	T_RTTI_CLASS;

public:
	ExtrudeShapeLayer(const ExtrudeShapeLayerData* data);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	virtual void pathChanged(const TransformPath& path) override final;

private:
	Ref< const ExtrudeShapeLayerData > m_data;
};

	}
}

/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
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

namespace traktor::mesh
{

class MeshAsset;

}

namespace traktor::shape
{

class CloneShapeLayerData;

/*!
 * \ingroup Shape
 */
class T_DLLCLASS CloneShapeLayer : public SplineLayerComponent
{
	T_RTTI_CLASS;

public:
	explicit CloneShapeLayer(
		const CloneShapeLayerData* data,
		mesh::MeshAsset* mesh,
		const model::Model* model
	);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	virtual Ref< model::Model > createModel(const TransformPath& path, bool closed, bool preview) const override final;

private:
	Ref< const CloneShapeLayerData > m_data;
	Ref< mesh::MeshAsset > m_mesh;
	Ref< const model::Model > m_model;
};

}

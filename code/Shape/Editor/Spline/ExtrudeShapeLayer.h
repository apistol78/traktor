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

class ExtrudeShapeLayerData;

/*!
 * \ingroup Shape
 */
class T_DLLCLASS ExtrudeShapeLayer : public SplineLayerComponent
{
	T_RTTI_CLASS;

public:
	explicit ExtrudeShapeLayer(
		const ExtrudeShapeLayerData* data,
		mesh::MeshAsset* meshStart, const model::Model* modelStart,
		mesh::MeshAsset* meshRepeat, const model::Model* modelRepeat,
		mesh::MeshAsset* meshEnd, const model::Model* modelEnd
	);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	virtual Ref< model::Model > createModel(const TransformPath& path, bool preview) const override final;

private:
	Ref< const ExtrudeShapeLayerData > m_data;

	Ref< mesh::MeshAsset > m_meshStart;
	Ref< const model::Model > m_modelStart;

	Ref< mesh::MeshAsset > m_meshRepeat;
	Ref< const model::Model > m_modelRepeat;

	Ref< mesh::MeshAsset > m_meshEnd;
	Ref< const model::Model > m_modelEnd;
};

}

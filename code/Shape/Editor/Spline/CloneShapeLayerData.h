#pragma once
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
#include "Resource/Id.h"
#include "Shape/Editor/Spline/SplineLayerComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::model
{
	
class Model;

}

namespace traktor::render
{

class Shader;

}

namespace traktor::shape
{

/*!
 * \ingroup Shape
 */
class T_DLLCLASS CloneShapeLayerData : public SplineLayerComponentData
{
	T_RTTI_CLASS;

public:
	CloneShapeLayerData();

	virtual Ref< SplineLayerComponent > createComponent(db::Database* database, const Path& modelCachePath, const std::wstring& assetPath) const override final;

	virtual void serialize(ISerializer& s) override final;

	const Guid& getMesh() const { return m_mesh; }

private:
	friend class CloneShapeLayer;

	Guid m_mesh;
	bool m_automaticOrientation;
	float m_distance;
	float m_startEndOffset;
	mutable Ref< model::Model > m_model;
};

}

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
#include "Resource/Id.h"
#include "Shape/Editor/Spline/SplineLayerComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace model
	{
	
class Model;

	}

	namespace render
	{

class Shader;

	}

	namespace shape
	{

/*!
 * \ingroup Shape
 */
class T_DLLCLASS ExtrudeShapeLayerData : public SplineLayerComponentData
{
	T_RTTI_CLASS;

public:
	ExtrudeShapeLayerData();

	virtual Ref< SplineLayerComponent > createComponent(db::Database* database) const override final;

	virtual Ref< model::Model > createModel(db::Database* database, model::ModelCache* modelCache, const std::wstring& assetPath, const TransformPath& path) const override final;

	virtual void serialize(ISerializer& s) override final;

	const Guid& getMeshStart() const { return m_meshStart; }

	const Guid& getMeshRepeat() const { return m_meshRepeat; }

	const Guid& getMeshEnd() const { return m_meshEnd; }

private:
	Guid m_meshStart;
	Guid m_meshRepeat;
	Guid m_meshEnd;
	bool m_automaticOrientation;
	float m_detail;
	mutable Ref< model::Model > m_modelStart;
	mutable Ref< model::Model > m_modelRepeat;	// \fixme Caching this here isn't pretty.
	mutable Ref< model::Model > m_modelEnd;
};

	}
}

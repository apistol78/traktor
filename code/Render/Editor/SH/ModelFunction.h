/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Render/SH/SHFunction.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class SahTree;

}

namespace traktor::model
{

class Model;

}

namespace traktor::render
{

class T_DLLCLASS ModelFunction : public SHFunction
{
	T_RTTI_CLASS;

public:
	explicit ModelFunction(model::Model* model);

	void setEvaluationPoint(const Vector4& position, const Vector4& normal);

	virtual Vector4 evaluate(const Polar& direction) const override final;

private:
	Ref< model::Model > m_model;
	Ref< SahTree > m_sahTree;
	Vector4 m_position;
	Vector4 m_normal;
};

}

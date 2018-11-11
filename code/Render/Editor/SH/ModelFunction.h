/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
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

	namespace model
	{

class Model;

	}

	namespace render
	{

class T_DLLCLASS ModelFunction : public SHFunction
{
	T_RTTI_CLASS;

public:
	ModelFunction(model::Model* model);

	void setEvaluationPoint(const Vector4& position, const Vector4& normal);

	virtual float evaluate(float phi, float theta, const Vector4& unit) const T_OVERRIDE T_FINAL;

private:
	Ref< model::Model > m_model;
	Ref< SahTree > m_sahTree;
	Vector4 m_position;
	Vector4 m_normal;
};

	}
}

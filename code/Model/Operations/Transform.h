/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_model_Transform_H
#define traktor_model_Transform_H

#include "Core/Math/Matrix44.h"
#include "Model/IModelOperation.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MODEL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace model
	{

/*! \brief
 * \ingroup Model
 */
class T_DLLCLASS Transform : public IModelOperation
{
	T_RTTI_CLASS;

public:
	Transform(const Matrix44& tf);

	virtual bool apply(Model& model) const T_OVERRIDE T_FINAL;

private:
	Matrix44 m_transform;
};

	}
}

#endif	// traktor_model_Transform_H

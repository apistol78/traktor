/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_model_ReduceSimplygon_H
#define traktor_model_ReduceSimplygon_H

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

#if defined(T_USE_SIMPLYGON_SDK)

/*! \brief
 * \ingroup Model
 */
class T_DLLCLASS ReduceSimplygon : public IModelOperation
{
	T_RTTI_CLASS;

public:
	ReduceSimplygon(float target);

	virtual bool apply(Model& model) const T_OVERRIDE T_FINAL;

private:
	float m_target;
};

#endif

	}
}

#endif	// traktor_model_ReduceSimplygon_H

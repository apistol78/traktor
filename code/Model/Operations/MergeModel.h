/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_model_MergeModel_H
#define traktor_model_MergeModel_H

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
class T_DLLCLASS MergeModel : public IModelOperation
{
	T_RTTI_CLASS;

public:
	MergeModel(const Model& sourceModel, const Transform& sourceTransform, float positionDistance);

	virtual bool apply(Model& model) const T_OVERRIDE T_FINAL;

private:
	const Model& m_sourceModel;
	Transform m_sourceTransform;
	float m_positionDistance;
};

	}
}

#endif	// traktor_model_MergeModel_H

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_model_MergeCoplanarAdjacents_H
#define traktor_model_MergeCoplanarAdjacents_H

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
class T_DLLCLASS MergeCoplanarAdjacents : public IModelOperation
{
	T_RTTI_CLASS;

public:
	MergeCoplanarAdjacents(bool allowConvexOnly);

	virtual bool apply(Model& model) const T_OVERRIDE T_FINAL;

private:
	bool m_allowConvexOnly;
};

	}
}

#endif	// traktor_model_MergeCoplanarAdjacents_H

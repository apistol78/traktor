/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_model_IModelOperation_H
#define traktor_model_IModelOperation_H

#include "Core/Object.h"

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

class Model;

/*! \brief
 * \ingroup Model
 */
class T_DLLCLASS IModelOperation : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool apply(Model& model) const = 0;
};

	}
}

#endif	// traktor_model_IModelOperation_H

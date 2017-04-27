/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_model_UnwrapUV_H
#define traktor_model_UnwrapUV_H

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
class T_DLLCLASS UnwrapUV : public IModelOperation
{
	T_RTTI_CLASS;

public:
	UnwrapUV(int32_t channel, float uvPerUnit, int32_t size, int32_t margin);

	virtual bool apply(Model& model) const T_OVERRIDE T_FINAL;

private:
	int32_t m_channel;
	float m_uvPerUnit;
	int32_t m_size;
	int32_t m_margin;
};

	}
}

#endif	// traktor_model_UnwrapUV_H

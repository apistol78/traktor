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
	UnwrapUV(int32_t channel, float margin);

	virtual bool apply(Model& model) const;

private:
	int32_t m_channel;
	float m_margin;
};

	}
}

#endif	// traktor_model_UnwrapUV_H

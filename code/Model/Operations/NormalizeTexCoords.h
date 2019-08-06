#pragma once

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
class T_DLLCLASS NormalizeTexCoords : public IModelOperation
{
	T_RTTI_CLASS;

public:
    NormalizeTexCoords(uint32_t channel, float marginU, float marginV);

	virtual bool apply(Model& model) const override final;

private:
    uint32_t m_channel;
    float m_marginU;
    float m_marginV;
};

	}
}


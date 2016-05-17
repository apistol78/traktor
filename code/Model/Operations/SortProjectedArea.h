#ifndef traktor_model_SortProjectedArea_H
#define traktor_model_SortProjectedArea_H

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
class T_DLLCLASS SortProjectedArea : public IModelOperation
{
	T_RTTI_CLASS;

public:
	SortProjectedArea(bool insideOut);

	virtual bool apply(Model& model) const T_OVERRIDE T_FINAL;

private:
	bool m_insideOut;
};

	}
}

#endif	// traktor_model_SortProjectedArea_H

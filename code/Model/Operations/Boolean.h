#ifndef traktor_model_Boolean_H
#define traktor_model_Boolean_H

#include "Core/Math/Transform.h"
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
class T_DLLCLASS Boolean : public IModelOperation
{
	T_RTTI_CLASS;

public:
	Boolean(
		const Model& modelA, const Transform& modelTransformA,
		const Model& modelB, const Transform& modelTransformB
	);

	virtual bool apply(Model& model) const;

private:
	const Model& m_modelA;
	const Model& m_modelB;
	Transform m_modelTransformA;
	Transform m_modelTransformB;
};

	}
}

#endif	// traktor_model_Boolean_H

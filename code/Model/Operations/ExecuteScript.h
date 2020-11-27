#pragma once

#include "Core/Ref.h"
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

class IRuntimeClass;

	namespace model
	{

/*! Execute script to process model.
 * \ingroup Model
 */
class T_DLLCLASS ExecuteScript : public IModelOperation
{
	T_RTTI_CLASS;

public:
    explicit ExecuteScript(const IRuntimeClass* scriptClass);

	virtual bool apply(Model& model) const override final;

private:
	Ref< const IRuntimeClass > m_scriptClass;
};

	}
}


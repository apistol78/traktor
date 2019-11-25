#pragma once

#include "Core/Serialization/ISerializable.h"

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

/*!
 * \ingroup Model
 */
class T_DLLCLASS IModelOperation : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual bool apply(Model& model) const = 0;
};

	}
}


#ifndef traktor_editor_ITypedAsset_H
#define traktor_editor_ITypedAsset_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

/*! \brief Asset with product type info.
 * \ingroup Editor
 */
class T_DLLCLASS ITypedAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfo* getOutputType() const = 0;
};

	}
}

#endif	// traktor_editor_ITypedAsset_H

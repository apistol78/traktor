#ifndef traktor_editor_VirtualPlaceholder_H
#define traktor_editor_VirtualPlaceholder_H

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

/*! \brief Virtual asset placeholder.
 * \ingroup Editor
 */
class T_DLLCLASS VirtualPlaceholder : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s);
};

	}
}

#endif	// traktor_editor_VirtualPlaceholder_H

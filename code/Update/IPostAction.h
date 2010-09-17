#ifndef traktor_update_IPostAction_H
#define traktor_update_IPostAction_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UPDATE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace update
	{

/*! \brief Post update action.
 * \ingroup Update
 */
class T_DLLCLASS IPostAction : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual bool execute() const = 0;
};

	}
}

#endif	// traktor_update_IPostAction_H

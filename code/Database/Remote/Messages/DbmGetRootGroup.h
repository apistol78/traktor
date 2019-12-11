#pragma once

#include "Database/Remote/IMessage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_REMOTE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

/*! Get database root group.
 * \ingroup Database
 */
class T_DLLCLASS DbmGetRootGroup : public IMessage
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;
};

	}
}


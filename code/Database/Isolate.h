#pragma once

#include "Core/Config.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

}

namespace traktor::db
{

class Group;
class Instance;

/*! Instance isolation.
 * \ingroup Database
 */
struct T_DLLCLASS Isolate
{
	static bool createIsolatedInstance(Instance* instance, IStream* stream);

	static Ref< Instance > createInstanceFromIsolation(Group* group, IStream* stream);
};

}

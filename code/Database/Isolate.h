#ifndef traktor_db_Isolate_H
#define traktor_db_Isolate_H

#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Stream;

	namespace db
	{

class Group;
class Instance;

/*! \brief Instance isolation.
 * \ingroup Database
 */
struct T_DLLCLASS Isolate
{
	static bool createIsolatedInstance(Instance* instance, Stream* stream);

	static Instance* createInstanceFromIsolation(Group* group, Stream* stream);
};

	}
}

#endif	// traktor_db_Isolate_H

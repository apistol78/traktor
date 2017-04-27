/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_db_Isolate_H
#define traktor_db_Isolate_H

#include "Core/Config.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

	namespace db
	{

class Group;
class Instance;

/*! \brief Instance isolation.
 * \ingroup Database
 */
struct T_DLLCLASS Isolate
{
	static bool createIsolatedInstance(Instance* instance, IStream* stream);

	static Ref< Instance > createInstanceFromIsolation(Group* group, IStream* stream);
};

	}
}

#endif	// traktor_db_Isolate_H

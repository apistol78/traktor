#ifndef traktor_db_IProviderDatabase_H
#define traktor_db_IProviderDatabase_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class IProviderBus;
class IProviderGroup;

/*! \brief Provider database interface.
 * \ingroup Database
 */
class T_DLLCLASS IProviderDatabase : public Object
{
	T_RTTI_CLASS;

public:
	virtual void close() = 0;

	virtual Ref< IProviderBus > getBus() = 0;

	virtual Ref< IProviderGroup > getRootGroup() = 0;
};

	}
}

#endif	// traktor_db_IProviderDatabase_H

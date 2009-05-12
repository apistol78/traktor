#ifndef traktor_db_LocalContext_H
#define traktor_db_LocalContext_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Core/Guid.h"

namespace traktor
{
	namespace db
	{

class DataAccess;

/*! \brief Local database context.
 * \ingroup Database
 */
class LocalContext : public Object
{
	T_RTTI_CLASS(LocalContext)

public:
	LocalContext(bool binary);

	const Guid& getSessionGuid() const;

	DataAccess* getDataAccess() const;

private:
	Guid m_sessionGuid;
	Ref< DataAccess > m_dataAccess;
};

	}
}

#endif	// traktor_db_LocalContext_H

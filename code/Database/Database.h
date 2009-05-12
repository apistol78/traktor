#ifndef traktor_db_Database_H
#define traktor_db_Database_H

#include <map>
#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Core/Guid.h"
#include "Core/Thread/Semaphore.h"
#include "Database/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Serializable;

	namespace db
	{

class IProviderDatabase;

class Group;
class Instance;

/*! \brief Database engine.
 * \ingroup Database
 *
 * The Database class manages a local view of
 * a database provider.
 */
class T_DLLCLASS Database : public Object
{
	T_RTTI_CLASS(Database)

public:
	virtual bool create(IProviderDatabase* providerDatabase);

	virtual void close();

	virtual Group* getRootGroup();

	virtual Group* getGroup(const std::wstring& groupPath);

	virtual Group* createGroup(const std::wstring& groupPath);

	virtual Instance* getInstance(const Guid& instanceGuid);

	virtual Instance* getInstance(const std::wstring& instancePath);

	virtual Instance* createInstance(const std::wstring& instancePath, Serializable* object, uint32_t flags = CifDefault, const Guid* guid = 0);

	virtual Serializable* getObjectReadOnly(const Guid& guid);

	template < typename T >
	T* getObjectReadOnly(const Guid& guid)
	{
		return dynamic_type_cast< T* >(getObjectReadOnly(guid));
	}

	/*! \brief Get event from bus.
	 *
	 * \note
	 * This method may flush database tree and should not be called
	 * from another thread if any instance or group is temporarily accessed.
	 *
	 * \param outEvent Event type.
	 * \param outEventId Event id, currently guid of affected instance.
	 * \param outRemote True if event originates from another connection; ie. another process.
	 * \return True if event was read from bus, false if no events are available.
	 */
	virtual bool getEvent(ProviderEvent& outEvent, Guid& outEventId, bool& outRemote);

private:
	Ref< IProviderDatabase > m_providerDatabase;
	Ref< Group > m_rootGroup;
	Semaphore m_lock;
	std::map< Guid, Ref< Instance > > m_instanceMap;
};

	}
}

#endif	// traktor_db_Database_H

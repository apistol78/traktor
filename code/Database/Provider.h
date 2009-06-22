#ifndef traktor_db_Provider_H
#define traktor_db_Provider_H

#include <string>
#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Core/Guid.h"
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

class Stream;
class Serializable;

	namespace db
	{

/*! \ingroup Database */
//@{

class IProviderBus;
class IProviderGroup;
class IProviderInstance;

/*! \brief Provider database interface. */
class T_DLLCLASS IProviderDatabase : public Object
{
	T_RTTI_CLASS(IProviderDatabase)

public:
	virtual void close() = 0;

	virtual IProviderBus* getBus() = 0;

	virtual IProviderGroup* getRootGroup() = 0;
};

/*! \brief Provider event bus interface.
 *
 * The bus is used to communicate changes made to other
 * peers which is connected to the same database.
 *
 * The high level database layer puts events when
 * for example an instance has been commits.
 */
class T_DLLCLASS IProviderBus : public Object
{
	T_RTTI_CLASS(IProviderBus)

public:
	virtual bool putEvent(ProviderEvent event, const Guid& eventId) = 0;

	/*! \brief Get event from bus.
	 *
	 * \param outEvent Event type.
	 * \param outEventId Event id, currently guid of affected instance.
	 * \param outRemote True if event originates from another connection; ie. another process.
	 * \return True if event was read from bus, false if no events are available.
	 */
	virtual bool getEvent(ProviderEvent& outEvent, Guid& outEventId, bool& outRemote) = 0;
};

class T_DLLCLASS IProviderGroup : public Object
{
	T_RTTI_CLASS(IProviderGroup)

public:
	virtual std::wstring getName() const = 0;

	virtual bool rename(const std::wstring& name) = 0;

	virtual bool remove() = 0;

	virtual IProviderGroup* createGroup(const std::wstring& groupName) = 0;

	virtual IProviderInstance* createInstance(const std::wstring& instanceName, const Guid& instanceGuid) = 0;

	virtual bool getChildGroups(RefArray< IProviderGroup >& outChildGroups) = 0;

	virtual bool getChildInstances(RefArray< IProviderInstance >& outChildInstances) = 0;
};

class T_DLLCLASS IProviderInstance : public Object
{
	T_RTTI_CLASS(IProviderInstance)

public:
	virtual std::wstring getPrimaryTypeName() const = 0;

	virtual bool beginTransaction() = 0;

	virtual bool endTransaction(bool commit) = 0;

	virtual std::wstring getName() const = 0;

	virtual bool setName(const std::wstring& name) = 0;

	virtual Guid getGuid() const = 0;

	virtual bool setGuid(const Guid& guid) = 0;

	virtual bool remove() = 0;

	virtual Serializable* getObject() = 0;

	virtual bool setObject(const Serializable* object) = 0;

	virtual uint32_t getDataNames(std::vector< std::wstring >& outDataNames) const = 0;

	virtual Stream* readData(const std::wstring& dataName) = 0;

	virtual Stream* writeData(const std::wstring& dataName) = 0;
};

//@}

	}
}

#endif	// traktor_db_Provider_H

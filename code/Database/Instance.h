#ifndef traktor_db_Instance_H
#define traktor_db_Instance_H

#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/Thread/Semaphore.h"
#include "Database/Config.h"
#include "Database/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;
class IStream;

	namespace db
	{

class IProviderBus;
class IProviderInstance;

class Database;
class Group;

/*! \brief Database instance.
 * \ingroup Database
 *
 * An instance is the actual container for data
 * inside the database.
 * Each instance reference to a primary object and any number
 * of attached data streams.
 */
class T_DLLCLASS Instance : public Object
{
	T_RTTI_CLASS;

public:
	Instance(Database* database);

	bool internalCreate(IProviderBus* providerBus, IProviderInstance* providerInstance, Group* parent);

	void internalDestroy();

	void internalReset();

	virtual std::wstring getName() const;

	virtual std::wstring getPath() const;

	virtual Guid getGuid() const;

	virtual bool setGuid(const Guid& guid);

	virtual std::wstring getPrimaryTypeName() const;

	virtual const TypeInfo* getPrimaryType() const;

	virtual bool checkout();

	virtual bool commit(uint32_t flags = CfDefault);

	virtual bool revert();

	virtual bool setName(const std::wstring& name);

	virtual bool remove();

	virtual Ref< ISerializable > getObject();

	virtual bool setObject(const ISerializable* object);

	virtual uint32_t getDataNames(std::vector< std::wstring >& dataNames) const;

	virtual bool removeAllData();

	virtual Ref< IStream > readData(const std::wstring& dataName);

	virtual Ref< IStream > writeData(const std::wstring& dataName);

	virtual Ref< Group > getParent() const;

	template < typename T >
	Ref< T > getObject()
	{
		return dynamic_type_cast< T* >(getObject());
	}

private:
	Database* m_database;
	IProviderBus* m_providerBus;
	Ref< IProviderInstance > m_providerInstance;
	Group* m_parent;
	mutable Semaphore m_lock;
	bool m_renamed;
	bool m_removed;
#if T_INSTANCE_CACHE_NAME || T_INSTANCE_CACHE_GUID || T_INSTANCE_CACHE_PRIMARY_TYPE
	mutable uint32_t m_cache;
#	if T_INSTANCE_CACHE_NAME
	mutable std::wstring m_name;
#	endif
#	if T_INSTANCE_CACHE_GUID
	mutable Guid m_guid;
#	endif
#	if T_INSTANCE_CACHE_PRIMARY_TYPE
	mutable std::wstring m_primaryType;
#	endif
#endif
};

	}
}

#endif	// traktor_db_Instance_H

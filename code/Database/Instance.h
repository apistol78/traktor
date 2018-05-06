/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_db_Instance_H
#define traktor_db_Instance_H

#include <vector>
#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Date/DateTime.h"
#include "Core/Thread/Semaphore.h"
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

class IInstanceEventListener;
class IProviderInstance;

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
	// \name Read-only queries.
	// \brief
	// Read-only queries can be performed without a transaction
	// and should return result as from latest successfully committed
	// transaction.
	// \{

	virtual Group* getParent() const;

	virtual std::wstring getName() const;

	virtual std::wstring getPath() const;

	virtual Guid getGuid() const;

	virtual bool getLastModifyDate(DateTime& outModifyDate) const;

	virtual uint32_t getFlags() const;

	virtual std::wstring getPrimaryTypeName() const;

	virtual const TypeInfo* getPrimaryType() const;

	virtual Ref< ISerializable > getObject() const;

	virtual uint32_t getDataNames(std::vector< std::wstring >& dataNames) const;

	virtual bool getDataLastWriteTime(const std::wstring& dataName, DateTime& outLastWriteTime) const;

	virtual Ref< IStream > readData(const std::wstring& dataName) const;

	template < typename T >
	Ref< T > getObject() const
	{
		return dynamic_type_cast< T* >(getObject());
	}

	// \}

	// \name Write queries; must be performed in a transaction using checkout/commit.
	// \{

	virtual bool checkout();

	virtual bool commit(uint32_t flags = CfDefault);

	virtual bool revert();

	virtual bool remove();

	virtual bool setName(const std::wstring& name);

	virtual bool setGuid(const Guid& guid);

	virtual bool setObject(const ISerializable* object);

	virtual bool removeAllData();

	virtual Ref< IStream > writeData(const std::wstring& dataName);

	// \}

private:
	friend class Database;
	friend class Group;

	IInstanceEventListener* m_eventListener;
	Ref< IProviderInstance > m_providerInstance;
	Group* m_parent;

	mutable Semaphore m_lock;
	mutable uint32_t m_cachedFlags;
	mutable std::wstring m_name;
	mutable Guid m_guid;
	mutable std::wstring m_type;

	Guid m_transactionGuid;
	std::wstring m_transactionName;
	uint32_t m_transactionFlags;

	Instance(IInstanceEventListener* eventListener);

	bool internalCreateExisting(IProviderInstance* providerInstance, Group* parent);

	bool internalCreateNew(IProviderInstance* providerInstance, Group* parent);

	void internalDestroy();

	void internalFlush();
};

	}
}

#endif	// traktor_db_Instance_H

#ifndef traktor_db_Instance_H
#define traktor_db_Instance_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Core/Thread/Mutex.h"
#include "Database/Config.h"
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
class Stream;

	namespace db
	{

class IProviderBus;
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
	T_RTTI_CLASS(Instance)

public:
	Instance(IProviderBus* providerBus);

	bool internalCreate(IProviderInstance* providerInstance, Group* parent);

	void internalDestroy();

	virtual std::wstring getName() const;

	virtual std::wstring getPath() const;

	virtual Guid getGuid() const;

	virtual std::wstring getPrimaryTypeName() const;

	virtual const Type* getPrimaryType() const;

	virtual bool checkout();

	virtual bool commit(uint32_t flags = CfDefault);

	virtual bool revert();

	virtual bool setName(const std::wstring& name);

	virtual bool remove();

	virtual Ref< Serializable > getObject();

	virtual bool setObject(const Serializable* object);

	virtual uint32_t getDataNames(std::vector< std::wstring >& dataNames) const;

	virtual Ref< Stream > readData(const std::wstring& dataName);

	virtual Ref< Stream > writeData(const std::wstring& dataName);

	virtual Ref< Group > getParent() const;

	template < typename T >
	Ref< T > getObject()
	{
		return dynamic_type_cast< T* >(getObject());
	}

private:
	Ref< IProviderBus > m_providerBus;
	Ref< IProviderInstance > m_providerInstance;
	Ref< Group > m_parent;
	Mutex m_lock;
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

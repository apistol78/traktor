#ifndef traktor_db_Instance_H
#define traktor_db_Instance_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Core/Thread/Mutex.h"
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

	bool internalCreate(IProviderInstance* providerInstance, Group* parent, Serializable* exclusiveObject);

	void internalDestroy();

	virtual std::wstring getName() const;

	virtual std::wstring getPath() const;

	virtual Guid getGuid() const;

	virtual std::wstring getPrimaryTypeName() const;

	virtual const Type* getPrimaryType() const;

	virtual bool lock();

	virtual bool unlock();

	virtual bool rename(const std::wstring& name);

	virtual bool remove();

	virtual Serializable* checkout(uint32_t flags = CfExclusive);

	virtual bool replace(Serializable* object);

	virtual bool commit(uint32_t flags = CfDefault);

	virtual bool revert();

	virtual uint32_t getDataNames(std::vector< std::wstring >& dataNames) const;

	virtual Stream* readData(const std::wstring& dataName);

	virtual Stream* writeData(const std::wstring& dataName);

	virtual Group* getParent();

	template < typename T >
	T* checkout(uint32_t flags = CfExclusive)
	{
		Ref< Serializable > object = checkout(flags);
		if (!object)
			return 0;

		if (!is_a< T >(object))
		{
			if (flags & CfExclusive)
				revert();
			return 0;
		}

		return reinterpret_cast< T* >(object.getPtr());
	}

private:
	Ref< IProviderBus > m_providerBus;
	Ref< IProviderInstance > m_providerInstance;
	Ref< Group > m_parent;
	std::wstring m_name;
	Guid m_guid;
	std::wstring m_primaryTypeName;
	Ref< Serializable > m_exclusiveObject;
	Mutex m_lock;
};

	}
}

#endif	// traktor_db_Instance_H

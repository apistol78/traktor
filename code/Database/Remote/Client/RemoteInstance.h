#ifndef traktor_db_RemoteInstance_H
#define traktor_db_RemoteInstance_H

#include "Core/Heap/Ref.h"
#include "Database/Provider/IProviderInstance.h"

namespace traktor
{
	namespace db
	{

class Connection;

/*! \brief Remote instance.
 * \ingroup Database
 */
class RemoteInstance : public IProviderInstance
{
	T_RTTI_CLASS(RemoteInstance)

public:
	RemoteInstance(Connection* connection, uint32_t handle);

	virtual std::wstring getPrimaryTypeName() const;

	virtual bool openTransaction();

	virtual bool commitTransaction();

	virtual bool closeTransaction();

	virtual std::wstring getName() const;

	virtual bool setName(const std::wstring& name);

	virtual Guid getGuid() const;

	virtual bool setGuid(const Guid& guid);

	virtual bool remove();

	virtual Stream* readObject(const Type*& outSerializerType);

	virtual Stream* writeObject(const std::wstring& primaryTypeName, const Type*& outSerializerType);

	virtual uint32_t getDataNames(std::vector< std::wstring >& outDataNames) const;

	virtual Stream* readData(const std::wstring& dataName);

	virtual Stream* writeData(const std::wstring& dataName);

private:
	Ref< Connection > m_connection;
	uint32_t m_handle;
};

	}
}

#endif	// traktor_db_RemoteInstance_H

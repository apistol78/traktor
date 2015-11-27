#ifndef traktor_db_RemoteInstance_H
#define traktor_db_RemoteInstance_H

#include "Database/Provider/IProviderInstance.h"
#include "Net/SocketAddressIPv4.h"

namespace traktor
{
	namespace db
	{

class RemoteConnection;

/*! \brief Remote instance.
 * \ingroup Database
 */
class RemoteInstance : public IProviderInstance
{
	T_RTTI_CLASS;

public:
	RemoteInstance(RemoteConnection* connection, uint32_t handle);

	virtual ~RemoteInstance();

	virtual std::wstring getPrimaryTypeName() const T_OVERRIDE T_FINAL;

	virtual bool openTransaction() T_OVERRIDE T_FINAL;

	virtual bool commitTransaction() T_OVERRIDE T_FINAL;

	virtual bool closeTransaction() T_OVERRIDE T_FINAL;

	virtual std::wstring getName() const T_OVERRIDE T_FINAL;

	virtual bool setName(const std::wstring& name) T_OVERRIDE T_FINAL;

	virtual Guid getGuid() const T_OVERRIDE T_FINAL;

	virtual bool setGuid(const Guid& guid) T_OVERRIDE T_FINAL;

	virtual bool getLastModifyDate(DateTime& outModifyDate) const T_OVERRIDE T_FINAL;

	virtual uint32_t getFlags() const T_OVERRIDE T_FINAL;

	virtual bool remove() T_OVERRIDE T_FINAL;

	virtual Ref< IStream > readObject(const TypeInfo*& outSerializerType) const T_OVERRIDE T_FINAL;

	virtual Ref< IStream > writeObject(const std::wstring& primaryTypeName, const TypeInfo*& outSerializerType) T_OVERRIDE T_FINAL;

	virtual uint32_t getDataNames(std::vector< std::wstring >& outDataNames) const T_OVERRIDE T_FINAL;

	virtual bool removeAllData() T_OVERRIDE T_FINAL;

	virtual bool getDataLastWriteTime(const std::wstring& dataName, DateTime& outLastWriteTime) const T_OVERRIDE T_FINAL;

	virtual Ref< IStream > readData(const std::wstring& dataName) const T_OVERRIDE T_FINAL;

	virtual Ref< IStream > writeData(const std::wstring& dataName) T_OVERRIDE T_FINAL;

private:
	Ref< RemoteConnection > m_connection;
	uint32_t m_handle;
};

	}
}

#endif	// traktor_db_RemoteInstance_H

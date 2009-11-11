#ifndef traktor_db_LocalInstance_H
#define traktor_db_LocalInstance_H

#include "Database/Provider/IProviderInstance.h"
#include "Core/Heap/Ref.h"
#include "Core/Io/Path.h"

namespace traktor
{
	namespace db
	{

class Context;
class LocalGroup;
class Transaction;

/*! \brief Local instance.
 * \ingroup Database
 */
class LocalInstance : public IProviderInstance
{
	T_RTTI_CLASS(LocalInstance)

public:
	LocalInstance(Context* context);

	bool internalCreate(const Path& instancePath);

	bool internalCreateNew(const Path& instancePath, const Guid& instanceGuid);

	virtual std::wstring getPrimaryTypeName() const;

	virtual bool openTransaction();

	virtual bool commitTransaction();

	virtual bool closeTransaction();

	virtual std::wstring getName() const;

	virtual bool setName(const std::wstring& name);

	virtual Guid getGuid() const;

	virtual bool setGuid(const Guid& guid);

	virtual bool remove();

	virtual Ref< Stream > readObject(const Type*& outSerializerType);

	virtual Ref< Stream > writeObject(const std::wstring& primaryTypeName, const Type*& outSerializerType);

	virtual uint32_t getDataNames(std::vector< std::wstring >& outDataNames) const;

	virtual Ref< Stream > readData(const std::wstring& dataName);

	virtual Ref< Stream > writeData(const std::wstring& dataName);

private:
	Ref< Context > m_context;
	Path m_instancePath;
	Ref< Transaction > m_transaction;
	std::wstring m_transactionName;
};

	}
}

#endif	// traktor_db_LocalInstance_H

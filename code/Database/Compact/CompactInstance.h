#ifndef traktor_db_CompactInstance_H
#define traktor_db_CompactInstance_H

#include "Database/Provider/IProviderInstance.h"

namespace traktor
{
	namespace db
	{

class CompactContext;
class CompactInstanceEntry;

/*! \brief Compact instance
 * \ingroup Database
 */
class CompactInstance : public IProviderInstance
{
	T_RTTI_CLASS;

public:
	CompactInstance(CompactContext* context);

	bool internalCreate(CompactInstanceEntry* instanceEntry);

	virtual std::wstring getPrimaryTypeName() const;

	virtual bool openTransaction();

	virtual bool commitTransaction();

	virtual bool closeTransaction();

	virtual std::wstring getName() const;

	virtual bool setName(const std::wstring& name);

	virtual Guid getGuid() const;

	virtual bool setGuid(const Guid& guid);

	virtual bool getLastModifyDate(DateTime& outModifyDate) const;

	virtual bool remove();

	virtual Ref< IStream > readObject(const TypeInfo*& outSerializerType) const;

	virtual Ref< IStream > writeObject(const std::wstring& primaryTypeName, const TypeInfo*& outSerializerType);

	virtual uint32_t getDataNames(std::vector< std::wstring >& outDataNames) const;

	virtual bool removeAllData();

	virtual Ref< IStream > readData(const std::wstring& dataName) const;

	virtual Ref< IStream > writeData(const std::wstring& dataName);

private:
	Ref< CompactContext > m_context;
	Ref< CompactInstanceEntry > m_instanceEntry;
};

	}
}

#endif	// traktor_db_CompactInstance_H

#pragma once

#include "Database/Provider/IProviderInstance.h"

namespace traktor
{
	namespace db
	{

class CompactContext;
class CompactInstanceEntry;

/*! Compact instance
 * \ingroup Database
 */
class CompactInstance : public IProviderInstance
{
	T_RTTI_CLASS;

public:
	explicit CompactInstance(CompactContext& context);

	bool internalCreate(CompactInstanceEntry* instanceEntry);

	virtual std::wstring getPrimaryTypeName() const override final;

	virtual bool openTransaction() override final;

	virtual bool commitTransaction() override final;

	virtual bool closeTransaction() override final;

	virtual std::wstring getName() const override final;

	virtual bool setName(const std::wstring& name) override final;

	virtual Guid getGuid() const override final;

	virtual bool setGuid(const Guid& guid) override final;

	virtual bool getLastModifyDate(DateTime& outModifyDate) const override final;

	virtual uint32_t getFlags() const override final;

	virtual bool remove() override final;

	virtual Ref< IStream > readObject(const TypeInfo*& outSerializerType) const override final;

	virtual Ref< IStream > writeObject(const std::wstring& primaryTypeName, const TypeInfo*& outSerializerType) override final;

	virtual uint32_t getDataNames(std::vector< std::wstring >& outDataNames) const override final;

	virtual bool removeAllData() override final;

	virtual bool getDataLastWriteTime(const std::wstring& dataName, DateTime& outLastWriteTime) const override final;

	virtual Ref< IStream > readData(const std::wstring& dataName) const override final;

	virtual Ref< IStream > writeData(const std::wstring& dataName) override final;

private:
	CompactContext& m_context;
	Ref< CompactInstanceEntry > m_instanceEntry;
};

	}
}


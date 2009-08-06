#ifndef traktor_db_CompactInstance_H
#define traktor_db_CompactInstance_H

#include "Database/Provider/IProviderInstance.h"
#include "Core/Heap/Ref.h"

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
	T_RTTI_CLASS(CompactInstance)

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

	virtual bool remove();

	virtual Stream* readObject(const Type*& outSerializerType);

	virtual Stream* writeObject(const std::wstring& primaryTypeName, const Type*& outSerializerType);

	virtual uint32_t getDataNames(std::vector< std::wstring >& outDataNames) const;

	virtual Stream* readData(const std::wstring& dataName);

	virtual Stream* writeData(const std::wstring& dataName);

private:
	Ref< CompactContext > m_context;
	Ref< CompactInstanceEntry > m_instanceEntry;
};

	}
}

#endif	// traktor_db_CompactInstance_H

#ifndef traktor_db_CompactInstance_H
#define traktor_db_CompactInstance_H

#include "Database/Provider.h"

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

	virtual std::wstring getName() const;

	virtual Guid getGuid() const;

	virtual std::wstring getPrimaryTypeName() const;

	virtual bool rename(const std::wstring& name);

	virtual bool remove();

	virtual bool lock();

	virtual bool unlock();

	virtual Serializable* readObject();

	virtual bool writeObject(Serializable* object);

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
